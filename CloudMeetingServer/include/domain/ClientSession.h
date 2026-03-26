#pragma once
#include <string>
#include <cstdint>
#include <netinet/in.h>

struct ClientSession
{
    std::string userId;
    std::string nickname;
    std::string roomId;
    int         tcpFd           = -1;
    uint32_t    numericId       = 0;   // UDP 包头中的 userId 字段
    bool        cameraOn        = false;
    bool        micOn           = false;
    bool        screenOn        = false;
    int64_t     lastHeartbeat   = 0;   // unix timestamp (seconds)
    sockaddr_in udpAudioAddr    {};    // 上行音频源地址（用于下行转发）
    sockaddr_in udpVideoAddr    {};    // 上行视频源地址
    bool        udpAudioKnown   = false;
    bool        udpVideoKnown   = false;
};
