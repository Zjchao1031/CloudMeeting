#include "server/EpollReactor.h"
#include "common/Logger.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <unordered_map>

static std::unordered_map<int, EpollReactor::Callback> g_callbacks;

EpollReactor::EpollReactor() {}
EpollReactor::~EpollReactor() { stop(); if (m_epollFd >= 0) ::close(m_epollFd); }

bool EpollReactor::init()
{
    m_epollFd = ::epoll_create1(0);
    return m_epollFd >= 0;
}

void EpollReactor::addFd(int fd, Callback cb)
{
    epoll_event ev{};
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev);
    g_callbacks[fd] = std::move(cb);
}

void EpollReactor::removeFd(int fd)
{
    ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, nullptr);
    g_callbacks.erase(fd);
}

void EpollReactor::setTimeoutCallback(TimeoutCallback cb)
{
    m_timeoutCb = std::move(cb);
}

void EpollReactor::run()
{
    m_running = true;
    epoll_event events[1024];
    while (m_running) {
        int n = ::epoll_wait(m_epollFd, events, 1024, 1000);
        if (m_timeoutCb) m_timeoutCb();
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            auto it = g_callbacks.find(fd);
            if (it != g_callbacks.end()) it->second(fd);
        }
    }
}

void EpollReactor::stop() { m_running = false; }
