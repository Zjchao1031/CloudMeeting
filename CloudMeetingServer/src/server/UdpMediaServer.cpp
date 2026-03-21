#include "server/UdpMediaServer.h"
#include "common/Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

static int bindUdp(uint16_t port)
{
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;
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

bool UdpMediaServer::start(uint16_t audioUp, uint16_t videoUp, uint16_t, uint16_t)
{
    m_audioUpFd = bindUdp(audioUp);
    m_videoUpFd = bindUdp(videoUp);
    if (m_audioUpFd < 0 || m_videoUpFd < 0) {
        Logger::error("UdpMediaServer bind failed"); return false;
    }
    Logger::info("UdpMediaServer ready, audioUp=" + std::to_string(audioUp)
                 + " videoUp=" + std::to_string(videoUp));
    // TODO: 启动独立线程接收并转发媒体包
    return true;
}

void UdpMediaServer::stop()
{
    if (m_audioUpFd >= 0) { ::close(m_audioUpFd); m_audioUpFd = -1; }
    if (m_videoUpFd >= 0) { ::close(m_videoUpFd); m_videoUpFd = -1; }
}
