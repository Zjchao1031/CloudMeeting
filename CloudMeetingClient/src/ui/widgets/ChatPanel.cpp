#include "ui/widgets/ChatPanel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTimer>
#include <functional>

// Key filter: intercept Enter in QTextEdit via callback (no Q_OBJECT needed)
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
    loadMockData();
}

void ChatPanel::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header
    auto *header = new QLabel("\u6587\u5b57\u804a\u5929", this);
    header->setStyleSheet(
        "color: #8888A8; font-size: 12px; font-weight: 600;"
        "padding: 8px 16px; background: #1E1E2E; border-bottom: 1px solid #2E2E44;");
    root->addWidget(header);

    // Scroll area for messages
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

    // Separator
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #2E2E44;");
    root->addWidget(sep);

    // Input area
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

    // Enter key filter via lambda callback
    m_inputEdit->installEventFilter(
        new InputKeyFilter([this]() { onSendClicked(); }, this));
}

void ChatPanel::loadMockData()
{
    appendMessage("u001", "\u5f20\u4e09",
        "\u5927\u5bb6\u597d\uff0c\u6b22\u8fce\u53c2\u52a0\u672c\u6b21\u4f1a\u8bae\uff01");
    appendMessage("u002", "\u674e\u56db",
        "\u4f60\u597d\uff01\u6211\u5df2\u5c31\u7eea\u3002");
    appendMessage("u003", "\u738b\u4e94",
        "\u6536\u5230\uff0c\u6211\u8fd9\u8fb9\u7f51\u7edc\u6b63\u5e38\u3002");
    appendMessage("u001", "\u5f20\u4e09",
        "\u4eca\u5929\u4e3b\u8981\u8ba8\u8bba Q2 \u8ba1\u5212\uff0c\u9884\u8ba1 30 \u5206\u949f\u3002");
    appendMessage("u002", "\u674e\u56db",
        "\u597d\u7684\uff0c\u6211\u5df2\u7ecf\u51c6\u5907\u597d\u76f8\u5173\u6750\u6599\u4e86\u3002");
}

void ChatPanel::appendMessage(const QString &userId, const QString &nickname,
                              const QString &content)
{
    ChatMessageData msg;
    msg.userId   = userId;
    msg.nickname = nickname;
    msg.content  = content;

    int idx = m_messagesLayout->count() - 1; // before stretch
    m_messagesLayout->insertWidget(idx, makeBubble(msg));
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
    appendMessage("self", "\u6211", text);
    m_inputEdit->clear();
}

void ChatPanel::scrollToBottom()
{
    auto *sb = m_scrollArea->verticalScrollBar();
    sb->setValue(sb->maximum());
}

QWidget* ChatPanel::makeBubble(const ChatMessageData &msg)
{
    auto *bubble = new QWidget(m_messagesContainer);
    bubble->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *row = new QHBoxLayout(bubble);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(10);
    row->setAlignment(Qt::AlignTop);

    // Avatar
    auto *avatarLabel = new QLabel(bubble);
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setPixmap(makeAvatarPix(msg.nickname, 40));
    avatarLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    row->addWidget(avatarLabel, 0, Qt::AlignTop);

    // Right column
    auto *col = new QVBoxLayout;
    col->setSpacing(4);
    col->setContentsMargins(0, 0, 0, 0);

    auto *nickLabel = new QLabel(msg.nickname, bubble);
    nickLabel->setStyleSheet("color: #4F8EF7; font-size: 12px; font-weight: 600;");
    col->addWidget(nickLabel);

    auto *contentLabel = new QLabel(msg.content, bubble);
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet(
        "background: #2A2A3E; color: #E8E8F0; font-size: 13px;"
        "border-radius: 8px; padding: 8px 12px;");
    contentLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    col->addWidget(contentLabel);

    row->addLayout(col);
    return bubble;
}
