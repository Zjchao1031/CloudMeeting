#include "server/UdpMediaServer.h"
#include "service/MediaForwardService.h"
#include "protocol/MediaPacket.h"
#include "common/Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

static int bindUdp(uint16_t port)
{
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    // 设置接收超时 500ms，使 recvfrom 定期返回以检查 m_running
    struct timeval tv{0, 500000};
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);
    if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(fd); return -1;
    }
    return fd;
}

UdpMediaServer::UdpMediaServer() {}
UdpMediaServer::~UdpMediaServer() { stop(); }

bool UdpMediaServer::start(uint16_t audioUp, uint16_t videoUp,
                            uint16_t audioDn, uint16_t videoDn)
{
    m_audioUpFd = bindUdp(audioUp);
    m_videoUpFd = bindUdp(videoUp);
    m_audioDnFd = bindUdp(audioDn);
    m_videoDnFd = bindUdp(videoDn);

    if (m_audioUpFd < 0 || m_videoUpFd < 0 ||
        m_audioDnFd < 0 || m_videoDnFd < 0) {
        Logger::error("UdpMediaServer bind failed");
        stop();
        return false;
    }

    // 将下行 fd 注入 MediaForwardService
    MediaForwardService::instance().setAudioDnFd(m_audioDnFd);
    MediaForwardService::instance().setVideoDnFd(m_videoDnFd);

    m_running = true;
    m_audioThread = std::thread([this]{ audioRecvLoop(); });
    m_videoThread = std::thread([this]{ videoRecvLoop(); });

    Logger::info("UdpMediaServer ready  audioUp=" + std::to_string(audioUp)
                 + " videoUp=" + std::to_string(videoUp)
                 + " audioDn=" + std::to_string(audioDn)
                 + " videoDn=" + std::to_string(videoDn));
    return true;
}

void UdpMediaServer::stop()
{
    m_running = false;
    // 关闭 fd 以唤醒阻塞的 recvfrom
    if (m_audioUpFd >= 0) { ::close(m_audioUpFd); m_audioUpFd = -1; }
    if (m_videoUpFd >= 0) { ::close(m_videoUpFd); m_videoUpFd = -1; }
    if (m_audioDnFd >= 0) { ::close(m_audioDnFd); m_audioDnFd = -1; }
    if (m_videoDnFd >= 0) { ::close(m_videoDnFd); m_videoDnFd = -1; }
    if (m_audioThread.joinable()) m_audioThread.join();
    if (m_videoThread.joinable()) m_videoThread.join();
}

void UdpMediaServer::audioRecvLoop()
{
    constexpr size_t BUF_SIZE = 2048;
    uint8_t buf[BUF_SIZE];
    sockaddr_in srcAddr{};
    socklen_t   addrLen = sizeof(srcAddr);

    while (m_running) {
        ssize_t n = ::recvfrom(m_audioUpFd, buf, BUF_SIZE, 0,
                               reinterpret_cast<sockaddr*>(&srcAddr), &addrLen);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue; // 超时，重检 m_running
            break; // 真实错误
        }
        if (n == 0) break;
        if (static_cast<size_t>(n) < sizeof(AudioPacketHeader)) continue;

        uint32_t userId;
        std::memcpy(&userId, buf, sizeof(uint32_t)); // 包头首字段
        MediaForwardService::instance().forwardAudio(
            userId, buf, static_cast<size_t>(n), srcAddr);
    }
}

void UdpMediaServer::videoRecvLoop()
{
    constexpr size_t BUF_SIZE = 65536;
    auto buf = std::make_unique<uint8_t[]>(BUF_SIZE);
    sockaddr_in srcAddr{};
    socklen_t   addrLen = sizeof(srcAddr);

    while (m_running) {
        ssize_t n = ::recvfrom(m_videoUpFd, buf.get(), BUF_SIZE, 0,
                               reinterpret_cast<sockaddr*>(&srcAddr), &addrLen);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue; // 超时，重检 m_running
            break;
        }
        if (n == 0) break;
        if (static_cast<size_t>(n) < sizeof(VideoPacketHeader)) continue;

        uint32_t userId;
        std::memcpy(&userId, buf.get(), sizeof(uint32_t));
        MediaForwardService::instance().forwardVideo(
            userId, buf.get(), static_cast<size_t>(n), srcAddr);
    }
}
