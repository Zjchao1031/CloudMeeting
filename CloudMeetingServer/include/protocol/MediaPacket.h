#pragma once
#include <cstdint>

#pragma pack(push, 1)
// UDP 音频包头（10 字节）
struct AudioPacketHeader
{
    uint32_t userId;
    uint32_t timestamp;
    uint16_t sequence;
};

// UDP 视频包头（12 字节）
struct VideoPacketHeader
{
    uint32_t userId;
    uint32_t timestamp;
    uint16_t sequence;
    uint8_t  flags;      // bit0=IDR, bit1=分片起始, bit2=分片结束
    uint8_t  fragIndex;
};
#pragma pack(pop)
