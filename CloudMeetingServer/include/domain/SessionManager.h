#pragma once
#include "domain/ClientSession.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

class SessionManager
{
public:
    static SessionManager &instance();

    void            addSession(const ClientSession &session);
    void            removeSession(const std::string &userId);
    ClientSession  *findSession(const std::string &userId);
    ClientSession  *findSessionByFd(int fd);
    ClientSession  *findSessionByNumericId(uint32_t numId);
    void            updateHeartbeat(const std::string &userId);
    void            updateUdpAudioAddr(uint32_t numId, const sockaddr_in &addr);
    void            updateUdpVideoAddr(uint32_t numId, const sockaddr_in &addr);
    // 返回所有会话的副本（用于心跳超时扫描）
    std::vector<ClientSession> getAllSessions();

private:
    SessionManager() = default;
    std::unordered_map<std::string, ClientSession> m_sessions;
    std::mutex                                      m_mutex;
};
