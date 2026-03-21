#include "domain/SessionManager.h"
#include "common/TimeUtil.h"

SessionManager &SessionManager::instance()
{
    static SessionManager inst;
    return inst;
}

void SessionManager::addSession(const ClientSession &session)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessions[session.userId] = session;
}

void SessionManager::removeSession(const std::string &userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessions.erase(userId);
}

ClientSession *SessionManager::findSession(const std::string &userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sessions.find(userId);
    return it != m_sessions.end() ? &it->second : nullptr;
}

ClientSession *SessionManager::findSessionByFd(int fd)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto &[uid, s] : m_sessions)
        if (s.tcpFd == fd) return &s;
    return nullptr;
}

void SessionManager::updateHeartbeat(const std::string &userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sessions.find(userId);
    if (it != m_sessions.end())
        it->second.lastHeartbeat = TimeUtil::nowSeconds();
}
