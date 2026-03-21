#pragma once
#include <string>
#include <unordered_set>
#include <cstdint>

struct Room
{
    std::string              roomId;
    int                      maxMembers  = 10;
    bool                     hasPassword = false;
    std::string              password;
    std::string              hostUserId;
    std::unordered_set<std::string> memberIds;

    bool isFull()  const { return (int)memberIds.size() >= maxMembers; }
    bool isHost(const std::string &uid) const { return uid == hostUserId; }
};
