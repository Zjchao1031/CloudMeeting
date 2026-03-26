#pragma once
#include <functional>

// epoll 事件循环封装，监听 fd 的可读事件并回调
class EpollReactor
{
public:
    using Callback        = std::function<void(int fd)>;
    using TimeoutCallback = std::function<void()>;

    EpollReactor();
    ~EpollReactor();

    bool init();
    void addFd(int fd, Callback cb);
    void removeFd(int fd);
    // 设置超时回调：epoll_wait 返回后（包括超时）就会调用一次
    void setTimeoutCallback(TimeoutCallback cb);
    void run();   // 阻塞事件循环
    void stop();

private:
    int             m_epollFd      = -1;
    bool            m_running      = false;
    TimeoutCallback m_timeoutCb;
};
