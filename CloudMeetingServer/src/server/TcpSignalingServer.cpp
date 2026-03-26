#include "server/TcpSignalingServer.h"
#include "server/EpollReactor.h"
#include "handler/CreateRoomHandler.h"
#include "handler/JoinRoomHandler.h"
#include "handler/LeaveRoomHandler.h"
#include "handler/MediaStateHandler.h"
#include "handler/ChatMessageHandler.h"
#include "handler/HeartbeatHandler.h"
#include "handler/RequestKeyframeHandler.h"
#include "service/RoomService.h"
#include "domain/SessionManager.h"
#include "protocol/SignalCodec.h"
#include "common/Constants.h"
#include "common/TimeUtil.h"
#include "common/Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

TcpSignalingServer::TcpSignalingServer()
    : m_reactor(new EpollReactor)
{}

TcpSignalingServer::~TcpSignalingServer()
{
    stop();
    delete m_reactor;
}

void TcpSignalingServer::setNonBlocking(int fd)
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void TcpSignalingServer::registerHandlers()
{
    auto reg = [this](SignalType t, std::unique_ptr<ICommandHandler> h) {
        m_handlers[static_cast<uint8_t>(t)] = std::move(h);
    };
    reg(SignalType::CREATE_ROOM,      std::make_unique<CreateRoomHandler>());
    reg(SignalType::JOIN_ROOM,        std::make_unique<JoinRoomHandler>());
    reg(SignalType::LEAVE_ROOM,       std::make_unique<LeaveRoomHandler>());
    reg(SignalType::MEDIA_STATE,      std::make_unique<MediaStateHandler>());
    reg(SignalType::CHAT_MESSAGE,     std::make_unique<ChatMessageHandler>());
    reg(SignalType::HEARTBEAT,        std::make_unique<HeartbeatHandler>());
    reg(SignalType::REQUEST_KEYFRAME, std::make_unique<RequestKeyframeHandler>());
}

bool TcpSignalingServer::start(uint16_t port)
{
    m_listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0) { Logger::error("socket() failed"); return false; }

    int opt = 1;
    ::setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setNonBlocking(m_listenFd);

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (::bind(m_listenFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        Logger::error("bind() failed"); return false;
    }
    ::listen(m_listenFd, 128);

    if (!m_reactor->init()) { Logger::error("epoll_create failed"); return false; }

    registerHandlers();

    // 注册监听 fd：有新连接时调用 onAccept
    m_reactor->addFd(m_listenFd, [this](int) { onAccept(); });

    // 每次 epoll_wait 超时后检测心跳
    m_reactor->setTimeoutCallback([this]{ checkHeartbeats(); });

    // 在独立线程运行事件循环
    m_reactorThread = std::thread([this]{ m_reactor->run(); });

    Logger::info("TcpSignalingServer listening on port " + std::to_string(port));
    return true;
}

void TcpSignalingServer::stop()
{
    m_reactor->stop();
    if (m_reactorThread.joinable()) m_reactorThread.join();
    if (m_listenFd >= 0) { ::close(m_listenFd); m_listenFd = -1; }
}

void TcpSignalingServer::onAccept()
{
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t   len = sizeof(clientAddr);
        int clientFd = ::accept4(m_listenFd, (sockaddr*)&clientAddr, &len, SOCK_NONBLOCK);
        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            Logger::error("accept4 failed: " + std::string(strerror(errno)));
            break;
        }
        int flag = 1;
        ::setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

        m_recvBufs[clientFd] = {};
        m_reactor->addFd(clientFd, [this, clientFd](int) { onClientReadable(clientFd); });
        Logger::info("New client connected fd=" + std::to_string(clientFd));
    }
}

void TcpSignalingServer::onClientReadable(int fd)
{
    auto &buf = m_recvBufs[fd];
    uint8_t tmp[4096];

    // 边缘触发：循环读取直到 EAGAIN
    while (true) {
        ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
        if (n > 0) {
            buf.insert(buf.end(), tmp, tmp + n);
        } else if (n == 0) {
            onClientDisconnect(fd);
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            onClientDisconnect(fd);
            return;
        }
    }

    // 从缓冲区中循环解包
    size_t offset = 0;
    SignalPacket pkt;
    size_t consumed = 0;
    while (offset < buf.size() &&
           SignalCodec::decode(buf.data() + offset, buf.size() - offset, pkt, consumed)) {
        dispatchPacket(fd, pkt);
        offset += consumed;
    }
    buf.erase(buf.begin(), buf.begin() + static_cast<ptrdiff_t>(offset));
}

void TcpSignalingServer::dispatchPacket(int fd, const SignalPacket &pkt)
{
    auto it = m_handlers.find(static_cast<uint8_t>(pkt.type));
    if (it != m_handlers.end()) {
        it->second->handle(fd, pkt.payload);
    } else {
        Logger::warn("Unknown signal type: " + std::to_string(static_cast<int>(pkt.type)));
    }
}

void TcpSignalingServer::onClientDisconnect(int fd)
{
    Logger::info("Client disconnected fd=" + std::to_string(fd));
    m_reactor->removeFd(fd);
    m_recvBufs.erase(fd);
    RoomService::instance().leaveRoom(fd);
    ::close(fd);
}

void TcpSignalingServer::checkHeartbeats()
{
    auto sessions = SessionManager::instance().getAllSessions();
    for (const auto &s : sessions) {
        if (s.lastHeartbeat > 0 &&
            TimeUtil::isTimeout(s.lastHeartbeat, Constants::HEARTBEAT_TIMEOUT_SEC)) {
            Logger::warn("Heartbeat timeout: " + s.nickname + " (fd=" + std::to_string(s.tcpFd) + ")");
            onClientDisconnect(s.tcpFd);
        }
    }
}
