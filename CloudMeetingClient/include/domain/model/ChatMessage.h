#pragma once
#include <QString>
#include <QtGlobal>

/**
 * @file ChatMessage.h
 * @brief 定义聊天消息数据模型。
 */

/**
 * @struct ChatMessage
 * @brief 描述一条聊天消息的基础信息。
 */
struct ChatMessage
{
    QString userId;          ///< 发送者用户标识。
    QString nickname;        ///< 发送者显示昵称。
    QString content;         ///< 消息文本内容。
    qint64  timestamp = 0;   ///< 消息生成时间戳。
};
