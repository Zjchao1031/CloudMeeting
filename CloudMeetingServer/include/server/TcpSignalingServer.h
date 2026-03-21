#pragma once
#include <cstdint>

class EpollReactor;
class RoomManager;
class SessionManager;

class TcpSignalingServer
{
public:
    TcpSignalingServer();
    ~TcpSignalingServer();

    bool start(uint16_t port);
    void stop();

private:
    int            m_listenFd = -1;
    EpollReactor  *m_reactor  = nullptr;
};
