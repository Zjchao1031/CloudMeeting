#pragma once
#include <QString>
#include <QtGlobal>

struct ChatMessage
{
    QString userId;
    QString nickname;
    QString content;
    qint64  timestamp = 0;
};
