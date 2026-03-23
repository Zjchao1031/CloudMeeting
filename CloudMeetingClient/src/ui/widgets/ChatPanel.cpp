/**
 * @file ChatPanel.cpp
 * @brief 实现会议聊天面板组件。
 */
#include "ui/widgets/ChatPanel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTimer>
#include <functional>

/**
 * @class InputKeyFilter
 * @brief 拦截输入框回车键并触发发送回调的事件过滤器。
 */
class InputKeyFilter : public QObject
{
public:
    explicit InputKeyFilter(std::function<void()> onSend, QObject *parent = nullptr)
        : QObject(parent), m_onSend(std::move(onSend)) {}

    bool eventFilter(QObject *, QEvent *ev) override
    {
        if (ev->type() == QEvent::KeyPress) {
            auto *ke = static_cast<QKeyEvent*>(ev);
            if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                if (ke->modifiers() & Qt::ShiftModifier)
                    return false;
                if (m_onSend) m_onSend();
                return true;
            }
        }
        return false;
    }
private:
    std::function<void()> m_onSend;
};

/**
 * @brief 生成聊天消息头像占位图。
 * @param[in] initial 头像显示用的首字符文本。
 * @param[in] size 头像边长尺寸，单位：像素。
 * @return 生成的圆形头像图像。
 */
static QPixmap makeAvatarPix(const QString &initial, int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    int hue = initial.isEmpty() ? 200 : (initial.at(0).unicode() * 37) % 360;
    p.setBrush(QColor::fromHsv(hue, 120, 160));
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, size, size);
    p.setPen(Qt::white);
    QFont f = p.font();
    f.setPixelSize(size / 2);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter,
               initial.isEmpty() ? "?" : initial.left(1).toUpper());
    return pix;
}

ChatPanel::ChatPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void ChatPanel::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // 顶部标题区域。
    auto *header = new QLabel("文字聊天", this);
    header->setStyleSheet(
        "color: #8888A8; font-size: 12px; font-weight: 600;"
        "padding: 8px 16px; background: #1E1E2E; border-bottom: 1px solid #2E2E44;");
    root->addWidget(header);

    // 消息滚动区域。
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_messagesContainer = new QWidget(m_scrollArea);
    m_messagesContainer->setStyleSheet("background: transparent;");
    m_messagesLayout = new QVBoxLayout(m_messagesContainer);
    m_messagesLayout->setContentsMargins(8, 8, 8, 8);
    m_messagesLayout->setSpacing(12);
    m_messagesLayout->addStretch();
    m_scrollArea->setWidget(m_messagesContainer);
    root->addWidget(m_scrollArea, 1);

    // 输入区域分隔线。
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #2E2E44;");
    root->addWidget(sep);

    // 消息输入区域。
    auto *inputArea = new QWidget(this);
    inputArea->setStyleSheet("background: #1E1E2E;");
    auto *inputRow = new QHBoxLayout(inputArea);
    inputRow->setContentsMargins(8, 8, 8, 8);
    inputRow->setSpacing(8);

    m_inputEdit = new QTextEdit(inputArea);
    m_inputEdit->setPlaceholderText("\u8f93\u5165\u6d88\u606f... (Enter \u53d1\u9001)");
    m_inputEdit->setFixedHeight(72);
    m_inputEdit->setStyleSheet(
        "QTextEdit { background: #2A2A3E; color: #E8E8F0;"
        "  border: 1.5px solid #3A3A52; border-radius: 8px;"
        "  padding: 6px 10px; font-size: 13px; }"
        "QTextEdit:focus { border-color: #4F8EF7; }");

    auto *sendBtn = new QPushButton("\u53d1\u9001", inputArea);
    sendBtn->setObjectName("smallBtn");
    sendBtn->setFixedSize(60, 36);
    inputRow->addWidget(m_inputEdit);
    inputRow->addWidget(sendBtn, 0, Qt::AlignBottom);
    root->addWidget(inputArea);

    connect(sendBtn, &QPushButton::clicked, this, &ChatPanel::onSendClicked);

    // 通过事件过滤器支持回车发送。
    m_inputEdit->installEventFilter(
        new InputKeyFilter([this]() { onSendClicked(); }, this));
}

void ChatPanel::clearMessages()
{
    while (m_messagesLayout->count() > 1) {
        auto *item = m_messagesLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
}

void ChatPanel::appendMessage(const QString &userId, const QString &nickname,
                              const QString &content)
{
    Q_UNUSED(userId)
    int idx = m_messagesLayout->count() - 1; // 在尾部弹性项之前插入新消息。
    m_messagesLayout->insertWidget(idx, makeBubble(nickname, content));
    QTimer::singleShot(0, this, &ChatPanel::scrollToBottom);
}

void ChatPanel::onSendClicked()
{
    QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;

    if (text.toUtf8().size() > 1024) {
        m_inputEdit->setStyleSheet(
            "QTextEdit { background: #2A2A3E; color: #E8E8F0;"
            "  border: 1.5px solid #E05C5C; border-radius: 8px;"
            "  padding: 6px 10px; font-size: 13px; }");
        return;
    }
    m_inputEdit->setStyleSheet(
        "QTextEdit { background: #2A2A3E; color: #E8E8F0;"
        "  border: 1.5px solid #3A3A52; border-radius: 8px;"
        "  padding: 6px 10px; font-size: 13px; }"
        "QTextEdit:focus { border-color: #4F8EF7; }");

    emit messageSent(text);
    m_inputEdit->clear();
}

void ChatPanel::scrollToBottom()
{
    auto *sb = m_scrollArea->verticalScrollBar();
    sb->setValue(sb->maximum());
}

QWidget* ChatPanel::makeBubble(const QString &nickname, const QString &content)
{
    auto *bubble = new QWidget(m_messagesContainer);
    bubble->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *row = new QHBoxLayout(bubble);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(10);
    row->setAlignment(Qt::AlignTop);

    // 头像区域。
    auto *avatarLabel = new QLabel(bubble);
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setPixmap(makeAvatarPix(nickname, 40));
    avatarLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    row->addWidget(avatarLabel, 0, Qt::AlignTop);

    // 消息文本列。
    auto *col = new QVBoxLayout;
    col->setSpacing(4);
    col->setContentsMargins(0, 0, 0, 0);

    auto *nickLabel = new QLabel(nickname, bubble);
    nickLabel->setStyleSheet("color: #4F8EF7; font-size: 12px; font-weight: 600;");
    col->addWidget(nickLabel);

    auto *contentLabel = new QLabel(content, bubble);
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet(
        "background: #2A2A3E; color: #E8E8F0; font-size: 13px;"
        "border-radius: 8px; padding: 8px 12px;");
    contentLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    col->addWidget(contentLabel);

    row->addLayout(col);
    return bubble;
}
