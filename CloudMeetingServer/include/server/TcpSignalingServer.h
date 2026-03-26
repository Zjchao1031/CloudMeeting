#pragma once
#include "protocol/SignalPacket.h"
#include "handler/ICommandHandler.h"
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>
#include <thread>

class EpollReactor;

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
    std::thread    m_reactorThread;

    // fd -> 接收缓冲区（在 reactor 线程内单线程访问，无需锁）
    std::unordered_map<int, std::vector<uint8_t>> m_recvBufs;
    // 信令类型 -> handler 实例
    std::unordered_map<uint8_t, std::unique_ptr<ICommandHandler>> m_handlers;

    void registerHandlers();
    void onAccept();
    void onClientReadable(int fd);
    void dispatchPacket(int fd, const SignalPacket &pkt);
    void onClientDisconnect(int fd);
    void checkHeartbeats();       // 由 reactor 超时回调驱动，单线程安全

    static void setNonBlocking(int fd);
};
