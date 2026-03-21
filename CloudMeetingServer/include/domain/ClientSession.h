#pragma once
#include <string>
#include <cstdint>

struct ClientSession
{
    std::string userId;
    std::string nickname;
    std::string roomId;
    int         tcpFd       = -1;
    bool        cameraOn    = false;
    bool        micOn       = false;
    bool        screenOn    = false;
    int64_t     lastHeartbeat = 0; // unix timestamp (seconds)
};
