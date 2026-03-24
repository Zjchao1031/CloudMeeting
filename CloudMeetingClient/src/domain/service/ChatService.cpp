/**
 * @file ChatService.cpp
 * @brief 实现聊天消息业务服务。
 */
#include "domain/service/ChatService.h"
#include "network/NetworkFacade.h"
#include "common/Constants.h"

ChatService::ChatService(QObject *parent)
    : QObject(parent)
{}

void ChatService::setNetworkFacade(NetworkFacade *facade)
{
    m_network = facade;
    if (!m_network) return;
    connect(m_network, &NetworkFacade::chatBroadcast,
            this,      &ChatService::onChatBroadcast);
}

bool ChatService::validateMessageUtf8Bytes(const QString &text) const
{
    return text.toUtf8().size() <= Constants::CHAT_MAX_BYTES;
}

void ChatService::sendMessage(const QString &text)
{
    if (!validateMessageUtf8Bytes(text)) {
        emit sendFailed("消息过长，请限制在 1024 字节以内。");
        return;
    }

    // 阶段二暂用 Mock，阶段三再通过 NetworkFacade 发送。
    if (m_network) {
        m_network->sendChatMessage(text);
    }
}

void ChatService::appendIncomingMessage(const ChatMessage &msg)
{
    m_messages.append(msg);
    emit newMessageReceived(msg);
}

void ChatService::clearMessages()
{
    m_messages.clear();
}

const QVector<ChatMessage> &ChatService::messages() const
{
    return m_messages;
}

void ChatService::onChatBroadcast(QJsonObject payload)
{
    ChatMessage msg;
    msg.userId       = payload["user_id"].toString();
    msg.nickname     = payload["nickname"].toString();
    msg.avatarBase64 = payload["avatar_base64"].toString();
    msg.content      = payload["content"].toString();
    msg.timestamp    = payload["timestamp"].toVariant().toLongLong();
    appendIncomingMessage(msg);
}
