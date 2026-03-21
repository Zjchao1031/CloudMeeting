#pragma once
#include <functional>

// epoll 事件循环封装，监听 fd 的可读事件并回调
class EpollReactor
{
public:
    using Callback = std::function<void(int fd)>;

    EpollReactor();
    ~EpollReactor();

    bool init();
    void addFd(int fd, Callback cb);
    void removeFd(int fd);
    void run();   // 阻塞事件循环
    void stop();

private:
    int  m_epollFd  = -1;
    bool m_running  = false;
};
