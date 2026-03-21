#include "domain/service/ChatService.h"
#include "common/Constants.h"

bool ChatService::validateMessageUtf8Bytes(const QString &text) const
{
    return text.toUtf8().size() <= Constants::CHAT_MAX_BYTES;
}

void ChatService::sendMessage(const QString &text)
{
    Q_UNUSED(text)
    // TODO: 调用 NetworkFacade 发送
}

void ChatService::appendIncomingMessage(const ChatMessage &msg)
{
    m_messages.append(msg);
}

const QVector<ChatMessage> &ChatService::messages() const
{
    return m_messages;
}
