#pragma once
#include <QtGlobal>
#include <QByteArray>

// UDP 音频包头（10 字节固定头）
#pragma pack(push, 1)
struct AudioPacketHeader
{
    quint32 userId;
    quint32 timestamp;
    quint16 sequence;
};

// UDP 视频包头（12 字节固定头）
struct VideoPacketHeader
{
    quint32 userId;
    quint32 timestamp;
    quint16 sequence;
    quint8  flags;      // bit0=IDR, bit1=分片起始, bit2=分片结束
    quint8  fragIndex;
};
#pragma pack(pop)
