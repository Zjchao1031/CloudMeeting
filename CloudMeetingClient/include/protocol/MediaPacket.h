#pragma once
#include <QtGlobal>
#include <QByteArray>

/**
 * @file MediaPacket.h
 * @brief 定义媒体传输使用的数据包头结构。
 */

#pragma pack(push, 1)

/**
 * @struct AudioPacketHeader
 * @brief 描述 UDP 音频包固定头。
 */
struct AudioPacketHeader
{
    quint32 userId;    ///< 音频数据所属用户标识。
    quint32 timestamp; ///< 音频帧时间戳。
    quint16 sequence;  ///< 音频包顺序号。
};

/**
 * @struct VideoPacketHeader
 * @brief 描述 UDP 视频包固定头。
 */
struct VideoPacketHeader
{
    quint32 userId;    ///< 视频数据所属用户标识。
    quint32 timestamp; ///< 视频帧时间戳。
    quint16 sequence;  ///< 视频包顺序号。
    quint8  flags;     ///< 标志位：bit0 表示 IDR，bit1 表示分片起始，bit2 表示分片结束。
    quint8  fragIndex; ///< 当前视频分片索引。
};

#pragma pack(pop)
