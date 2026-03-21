#pragma once
#include <cstdint>

class UdpMediaServer
{
public:
    UdpMediaServer();
    ~UdpMediaServer();

    bool start(uint16_t audioUpPort, uint16_t videoUpPort,
               uint16_t audioDnPort, uint16_t videoDnPort);
    void stop();

private:
    int m_audioUpFd = -1;
    int m_videoUpFd = -1;
};
