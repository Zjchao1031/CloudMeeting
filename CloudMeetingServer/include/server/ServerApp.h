#pragma once

class TcpSignalingServer;
class UdpMediaServer;

class ServerApp
{
public:
    ServerApp();
    ~ServerApp();
    void run();
    void stop();

private:
    TcpSignalingServer *m_tcpServer = nullptr;
    UdpMediaServer     *m_udpServer = nullptr;
};
