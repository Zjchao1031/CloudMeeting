#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QScrollArea>

/**
 * @file ChatPanel.h
 * @brief 声明会议聊天面板组件。
 */

/**
 * @class ChatPanel
 * @brief 提供消息展示与发送输入能力的聊天侧边栏。
 */
class ChatPanel : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造聊天面板。
     * @param[in] parent 父窗口指针。
     */
    explicit ChatPanel(QWidget *parent = nullptr);

    /**
     * @brief 向聊天面板追加一条消息。
     * @param[in] userId 发送者用户标识。
     * @param[in] nickname 发送者显示昵称。
     * @param[in] content 消息文本内容。
     */
    void appendMessage(const QString &userId, const QString &nickname, const QString &content, const QString &avatarBase64 = {});

    /**
     * @brief 清空所有聊天消息。
     */
    void clearMessages();

signals:
    /**
     * @brief 当用户提交消息时发出该信号。
     * @param[in] text 待发送的消息文本。
     */
    void messageSent(const QString &text);

private slots:
    /**
     * @brief 处理发送按钮点击事件。
     */
    void onSendClicked();

private:
    /**
     * @brief 初始化聊天面板界面。
     */
    void setupUi();

    /**
     * @brief 根据消息数据创建单条消息气泡控件。
     * @param[in] nickname 发送者昵称。
     * @param[in] content 消息内容。
     * @return 消息气泡控件指针。
     */
    QWidget* makeBubble(const QString &nickname, const QString &content, const QString &avatarBase64);

    /**
     * @brief 将消息滚动区域滚动到最底部。
     */
    void scrollToBottom();

    QVBoxLayout *m_messagesLayout    = nullptr; ///< 消息列表布局。
    QWidget     *m_messagesContainer = nullptr; ///< 消息列表容器。
    QScrollArea *m_scrollArea        = nullptr; ///< 消息滚动区域。
    QTextEdit   *m_inputEdit         = nullptr; ///< 消息输入框。
};
