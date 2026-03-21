#include "ui/widgets/ChatPanel.h"

ChatPanel::ChatPanel(QWidget *parent)
    : QWidget(parent)
{}

void ChatPanel::appendMessage(const QString &userId, const QString &nickname, const QString &content)
{
    Q_UNUSED(userId)
    Q_UNUSED(nickname)
    Q_UNUSED(content)
}
