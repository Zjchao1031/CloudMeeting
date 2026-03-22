#pragma once
#include <QtGlobal>

/**
 * @file SignalType.h
 * @brief 定义客户端与服务端之间的信令消息类型。
 */

/**
 * @enum SignalType
 * @brief 表示信令协议中支持的消息类型。
 */
enum class SignalType : quint8
{
    CREATE_ROOM      = 0x01, ///< 创建会议请求。
    CREATE_ROOM_ACK  = 0x02, ///< 创建会议响应。
    JOIN_ROOM        = 0x03, ///< 加入会议请求。
    JOIN_ROOM_ACK    = 0x04, ///< 加入会议响应。
    LEAVE_ROOM       = 0x05, ///< 离开会议通知。
    ROOM_CLOSED      = 0x06, ///< 房间关闭通知。
    MEMBER_JOIN      = 0x07, ///< 成员加入通知。
    MEMBER_LEAVE     = 0x08, ///< 成员离开通知。
    MEDIA_STATE      = 0x09, ///< 媒体状态上报。
    MEDIA_STATE_SYNC = 0x0A, ///< 媒体状态同步。
    CHAT_MESSAGE     = 0x0B, ///< 聊天消息发送。
    CHAT_BROADCAST   = 0x0C, ///< 聊天消息广播。
    HEARTBEAT        = 0x0D, ///< 心跳请求。
    HEARTBEAT_ACK    = 0x0E, ///< 心跳响应。
    REQUEST_KEYFRAME = 0x0F, ///< 请求关键帧。
};
