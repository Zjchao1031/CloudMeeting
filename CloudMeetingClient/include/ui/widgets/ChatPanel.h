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
 * @struct ChatMessageData
 * @brief 描述聊天面板中展示用的消息数据。
 */
struct ChatMessageData {
    QString userId;           ///< 发送者用户标识。
    QString nickname;         ///< 发送者显示昵称。
    QString content;          ///< 消息文本内容。
    bool    isSelf = false;   ///< 是否为当前用户发送的消息。
};

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
    void appendMessage(const QString &userId, const QString &nickname, const QString &content);

    /**
     * @brief 加载用于界面演示的模拟聊天数据。
     */
    void loadMockData();

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
     * @param[in] msg 待展示的消息数据。
     * @return 消息气泡控件指针。
     */
    QWidget* makeBubble(const ChatMessageData &msg);

    /**
     * @brief 将消息滚动区域滚动到最底部。
     */
    void scrollToBottom();

    QVBoxLayout *m_messagesLayout    = nullptr; ///< 消息列表布局。
    QWidget     *m_messagesContainer = nullptr; ///< 消息列表容器。
    QScrollArea *m_scrollArea        = nullptr; ///< 消息滚动区域。
    QTextEdit   *m_inputEdit         = nullptr; ///< 消息输入框。
};
