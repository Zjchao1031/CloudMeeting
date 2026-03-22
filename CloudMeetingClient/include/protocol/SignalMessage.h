#pragma once
#include "protocol/SignalType.h"
#include <QJsonObject>

/**
 * @file SignalMessage.h
 * @brief 定义信令消息数据结构。
 */

/**
 * @struct SignalMessage
 * @brief 描述一条完整的信令消息。
 */
struct SignalMessage
{
    SignalType  type;    ///< 信令消息类型。
    QJsonObject payload; ///< 信令消息载荷。
};
