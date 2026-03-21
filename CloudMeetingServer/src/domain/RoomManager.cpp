#include "domain/RoomManager.h"
#include "common/IdGenerator.h"
#include <mutex>

RoomManager &RoomManager::instance()
{
    static RoomManager inst;
    return inst;
}

std::string RoomManager::createRoom(int maxMembers, bool hasPassword,
                                     const std::string &password,
                                     const std::string &hostId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string id = IdGenerator::generateRoomId();
    Room r;
    r.roomId      = id;
    r.maxMembers  = maxMembers;
    r.hasPassword = hasPassword;
    r.password    = password;
    r.hostUserId  = hostId;
    r.memberIds.insert(hostId);
    m_rooms[id]   = std::move(r);
    return id;
}

Room *RoomManager::findRoom(const std::string &roomId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_rooms.find(roomId);
    return it != m_rooms.end() ? &it->second : nullptr;
}

void RoomManager::destroyRoom(const std::string &roomId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rooms.erase(roomId);
}
