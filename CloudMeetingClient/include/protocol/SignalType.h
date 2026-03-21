#pragma once
#include <QtGlobal>

// 信令消息类型枚举（与服务端协议保持一致）
enum class SignalType : quint8
{
    CREATE_ROOM       = 0x01,
    CREATE_ROOM_ACK   = 0x02,
    JOIN_ROOM         = 0x03,
    JOIN_ROOM_ACK     = 0x04,
    LEAVE_ROOM        = 0x05,
    ROOM_CLOSED       = 0x06,
    MEMBER_JOIN       = 0x07,
    MEMBER_LEAVE      = 0x08,
    MEDIA_STATE       = 0x09,
    MEDIA_STATE_SYNC  = 0x0A,
    CHAT_MESSAGE      = 0x0B,
    CHAT_BROADCAST    = 0x0C,
    HEARTBEAT         = 0x0D,
    HEARTBEAT_ACK     = 0x0E,
    REQUEST_KEYFRAME  = 0x0F,
};
