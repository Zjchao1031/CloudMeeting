#pragma once
#include "domain/ClientSession.h"
#include <unordered_map>
#include <string>
#include <mutex>

class SessionManager
{
public:
    static SessionManager &instance();

    void            addSession(const ClientSession &session);
    void            removeSession(const std::string &userId);
    ClientSession  *findSession(const std::string &userId);
    ClientSession  *findSessionByFd(int fd);
    void            updateHeartbeat(const std::string &userId);

private:
    SessionManager() = default;
    std::unordered_map<std::string, ClientSession> m_sessions;
    std::mutex                                      m_mutex;
};
