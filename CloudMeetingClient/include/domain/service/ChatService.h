#pragma once
#include "domain/model/ChatMessage.h"
#include <QVector>
#include <QString>

/**
 * @file ChatService.h
 * @brief 声明聊天消息业务服务。
 */

/**
 * @class ChatService
 * @brief 负责聊天消息校验、发送与本地缓存管理。
 */
class ChatService
{
public:
    /**
     * @brief 校验消息的 UTF-8 字节长度是否合规。
     * @param[in] text 待校验的消息文本。
     * @return 若消息长度未超过限制则返回 `true`，否则返回 `false`。
     */
    bool validateMessageUtf8Bytes(const QString &text) const;

    /**
     * @brief 发送一条聊天消息。
     * @param[in] text 待发送的消息文本。
     */
    void sendMessage(const QString &text);

    /**
     * @brief 追加一条收到的聊天消息。
     * @param[in] msg 待加入本地缓存的消息对象。
     */
    void appendIncomingMessage(const ChatMessage &msg);

    /**
     * @brief 获取当前缓存的消息列表。
     * @return 聊天消息缓存的常量引用。
     */
    const QVector<ChatMessage> &messages() const;

private:
    QVector<ChatMessage> m_messages; ///< 按接收顺序保存的聊天消息列表。
};
