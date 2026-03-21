#pragma once
#include "domain/model/ChatMessage.h"
#include <QVector>
#include <QString>

class ChatService
{
public:
    bool validateMessageUtf8Bytes(const QString &text) const;
    void sendMessage(const QString &text);
    void appendIncomingMessage(const ChatMessage &msg);
    const QVector<ChatMessage> &messages() const;

private:
    QVector<ChatMessage> m_messages;
};
