#include "server/TcpSignalingServer.h"
#include "server/EpollReactor.h"
#include "common/Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

TcpSignalingServer::TcpSignalingServer()
    : m_reactor(new EpollReactor)
{}

TcpSignalingServer::~TcpSignalingServer()
{
    stop();
    delete m_reactor;
}

bool TcpSignalingServer::start(uint16_t port)
{
    m_listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0) { Logger::error("socket() failed"); return false; }

    int opt = 1;
    ::setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (::bind(m_listenFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        Logger::error("bind() failed"); return false;
    }
    ::listen(m_listenFd, 128);
    Logger::info("TcpSignalingServer listening on port " + std::to_string(port));
    // TODO: 将 m_listenFd 注册到 EpollReactor 并处理 accept
    return true;
}

void TcpSignalingServer::stop()
{
    if (m_listenFd >= 0) { ::close(m_listenFd); m_listenFd = -1; }
}
