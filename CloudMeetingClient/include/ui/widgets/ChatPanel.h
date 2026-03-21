#pragma once
#include <QWidget>
#include <QString>

class ChatPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ChatPanel(QWidget *parent = nullptr);
    void appendMessage(const QString &userId, const QString &nickname, const QString &content);

signals:
    void messageSent(const QString &text);
};
