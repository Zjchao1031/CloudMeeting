#pragma once
#include <QString>

/**
 * @file RoomInfo.h
 * @brief 定义会议房间信息模型。
 */

/**
 * @struct RoomInfo
 * @brief 描述当前会议房间的核心属性。
 */
struct RoomInfo
{
    QString roomId;              ///< 房间唯一标识。
    int     maxMembers  = 10;    ///< 房间允许的最大参会人数。
    bool    hasPassword = false;  ///< 房间是否启用密码。
    QString hostUserId;          ///< 主持人用户标识。
    bool    isHost      = false;  ///< 当前用户是否为主持人。
};
