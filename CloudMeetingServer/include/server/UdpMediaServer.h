#pragma once
#include <cstdint>
#include <thread>
#include <atomic>

class UdpMediaServer
{
public:
    UdpMediaServer();
    ~UdpMediaServer();

    bool start(uint16_t audioUpPort, uint16_t videoUpPort,
               uint16_t audioDnPort, uint16_t videoDnPort);
    void stop();

private:
    int m_audioUpFd = -1;   // 上行：接收客户端音频
    int m_videoUpFd = -1;   // 上行：接收客户端视频
    int m_audioDnFd = -1;   // 下行：向客户端发送音频
    int m_videoDnFd = -1;   // 下行：向客户端发送视频

    std::atomic<bool> m_running{false};
    std::thread       m_audioThread;
    std::thread       m_videoThread;

    void audioRecvLoop();
    void videoRecvLoop();
};
