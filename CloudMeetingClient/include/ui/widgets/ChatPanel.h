#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QScrollArea>

struct ChatMessageData {
    QString userId;
    QString nickname;
    QString content;
    bool    isSelf = false;
};

class ChatPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ChatPanel(QWidget *parent = nullptr);

    void appendMessage(const QString &userId, const QString &nickname, const QString &content);
    void loadMockData();

signals:
    void messageSent(const QString &text);

private slots:
    void onSendClicked();

private:
    void setupUi();
    QWidget* makeBubble(const ChatMessageData &msg);
    void scrollToBottom();

    QVBoxLayout *m_messagesLayout    = nullptr;
    QWidget     *m_messagesContainer = nullptr;
    QScrollArea *m_scrollArea        = nullptr;
    QTextEdit   *m_inputEdit         = nullptr;
};
