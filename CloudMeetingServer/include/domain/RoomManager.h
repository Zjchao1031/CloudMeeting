#pragma once
#include "domain/Room.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>

class RoomManager
{
public:
    static RoomManager &instance();

    std::string createRoom(int maxMembers, bool hasPassword, const std::string &password, const std::string &hostId);
    Room       *findRoom(const std::string &roomId);
    void        destroyRoom(const std::string &roomId);

private:
    RoomManager() = default;
    std::unordered_map<std::string, Room> m_rooms;
    std::mutex                             m_mutex;
};
