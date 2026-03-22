#include "ui/MainWindow.h"
#include "ui/MeetingWindow.h"
#include "ui/dialogs/UserProfileDialog.h"
#include "ui/dialogs/CreateMeetingDialog.h"
#include "ui/dialogs/JoinMeetingDialog.h"
#include "ui/dialogs/ConfirmDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QStackedWidget>
#include <QFrame>
#include <QApplication>
#include <QPolygon>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>

static QPixmap makeDefaultAvatar(int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor("#363650"));
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, size, size);
    p.setBrush(QColor("#8888A8"));
    int headR = size / 5;
    p.drawEllipse(size/2 - headR, size/5, headR*2, headR*2);
    return pix;
}

static QPixmap makeCircularPixmap(const QPixmap &src, int size)
{
    QPixmap scaled = src.scaled(size, size,
        Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRegion(QRegion(0, 0, size, size, QRegion::Ellipse));
    int ox = (scaled.width()  - size) / 2;
    int oy = (scaled.height() - size) / 2;
    p.drawPixmap(-ox, -oy, scaled);
    return result;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_avatar = makeDefaultAvatar(36);
    setupUi();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    setWindowTitle("CloudMeeting");
    resize(960, 640);
    setMinimumSize(800, 560);

    auto *stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // =========================================================
    // Page 0: Main / home page
    // =========================================================
    auto *mainPage = new QWidget(stack);
    mainPage->setStyleSheet("background: #1E1E2E;");

    auto *pageLayout = new QVBoxLayout(mainPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // ── Top bar ──
    auto *topBar = new QWidget(mainPage);
    topBar->setFixedHeight(60);
    topBar->setStyleSheet("background: #16162A; border-bottom: 1px solid #2A2A3E;");
    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 0, 20, 0);
    topBarLayout->setSpacing(0);

    // App name left
    auto *appNameLabel = new QLabel("CloudMeeting", topBar);
    appNameLabel->setStyleSheet(
        "color: #4F8EF7; font-size: 16px; font-weight: 700; letter-spacing: 1px;");
    topBarLayout->addWidget(appNameLabel);
    topBarLayout->addStretch();

    // User avatar + nickname (right) — 使用 QWidget + eventFilter 处理点击
    m_userContainer = new QWidget(topBar);
    m_userContainer->setStyleSheet(
        "QWidget#userContainer { background: transparent; border-radius: 8px; }"
        "QWidget#userContainer:hover { background: rgba(255,255,255,0.07); }"
    );
    m_userContainer->setObjectName("userContainer");
    m_userContainer->setCursor(Qt::PointingHandCursor);
    m_userContainer->installEventFilter(this);

    auto *userBtnLayout = new QHBoxLayout(m_userContainer);
    userBtnLayout->setContentsMargins(8, 4, 10, 4);
    userBtnLayout->setSpacing(8);

    m_avatarLabel = new QLabel(m_userContainer);
    m_avatarLabel->setFixedSize(32, 32);
    m_avatarLabel->setPixmap(makeCircularPixmap(m_avatar, 32));

    m_nicknameLabel = new QLabel(m_nickname, m_userContainer);
    m_nicknameLabel->setStyleSheet("color: #E8E8F0; font-size: 14px;");

    auto *chevron = new QLabel("\u25be", m_userContainer);
    chevron->setStyleSheet("color: #8888A8; font-size: 11px;");

    userBtnLayout->addWidget(m_avatarLabel);
    userBtnLayout->addWidget(m_nicknameLabel);
    userBtnLayout->addWidget(chevron);

    topBarLayout->addWidget(m_userContainer);

    pageLayout->addWidget(topBar);

    // ── Center hero ──
    auto *hero = new QWidget(mainPage);
    hero->setStyleSheet("background: transparent;");
    auto *heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(40, 0, 40, 40);
    heroLayout->setSpacing(0);
    heroLayout->setAlignment(Qt::AlignCenter);

    // Logo
    QPixmap logoPix(80, 80);
    logoPix.fill(Qt::transparent);
    {
        QPainter lp(&logoPix);
        lp.setRenderHint(QPainter::Antialiasing);
        lp.setBrush(QColor("#4F8EF7"));
        lp.setPen(Qt::NoPen);
        lp.drawRoundedRect(0, 0, 80, 80, 20, 20);
        // Camera body
        lp.setBrush(Qt::white);
        lp.drawRoundedRect(8, 24, 40, 30, 6, 6);
        // Lens ring
        lp.setBrush(QColor("#4F8EF7"));
        lp.drawEllipse(17, 29, 20, 20);
        lp.setBrush(Qt::white);
        lp.drawEllipse(22, 34, 10, 10);
        // Viewfinder
        QPolygon tri;
        tri << QPoint(52, 28) << QPoint(70, 20) << QPoint(70, 58) << QPoint(52, 50);
        lp.setBrush(Qt::white);
        lp.drawPolygon(tri);
    }
    auto *logoLabel = new QLabel(hero);
    logoLabel->setPixmap(logoPix);
    logoLabel->setAlignment(Qt::AlignCenter);
    heroLayout->addWidget(logoLabel);
    heroLayout->addSpacing(20);

    // Product name
    auto *productName = new QLabel("Cloud Meeting", hero);
    productName->setAlignment(Qt::AlignCenter);
    productName->setStyleSheet(
        "color: #E8E8F0; font-size: 30px; font-weight: 700; letter-spacing: 2px;"
    );
    heroLayout->addWidget(productName);
    heroLayout->addSpacing(8);

    auto *tagline = new QLabel("轻量 · 快速 · 免登录的实时音视频会议", hero);
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setStyleSheet("color: #8888A8; font-size: 14px;");
    heroLayout->addWidget(tagline);
    heroLayout->addSpacing(48);

    // Buttons
    auto *btnBox = new QWidget(hero);
    btnBox->setStyleSheet("background: transparent;");
    auto *btnLayout = new QVBoxLayout(btnBox);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(16);
    btnLayout->setAlignment(Qt::AlignCenter);

    auto *createBtn = new QPushButton("\u271a  创建会议", btnBox);
    createBtn->setObjectName("primaryBtn");
    createBtn->setFixedWidth(200);
    createBtn->setCursor(Qt::PointingHandCursor);

    auto *joinBtn = new QPushButton("\u2192  加入会议", btnBox);
    joinBtn->setObjectName("outlineBtn");
    joinBtn->setFixedWidth(200);
    joinBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(createBtn, 0, Qt::AlignCenter);
    btnLayout->addWidget(joinBtn,   0, Qt::AlignCenter);
    heroLayout->addWidget(btnBox);

    pageLayout->addWidget(hero, 1);
    stack->addWidget(mainPage);   // index 0

    connect(createBtn, &QPushButton::clicked, this, &MainWindow::onCreateMeetingClicked);
    connect(joinBtn,   &QPushButton::clicked, this, &MainWindow::onJoinMeetingClicked);

    // =========================================================
    // Page 1: Meeting window
    // =========================================================
    m_meetingWindow = new MeetingWindow(stack);
    stack->addWidget(m_meetingWindow);  // index 1

    connect(m_meetingWindow, &MeetingWindow::leaveRequested,
            this, &MainWindow::onLeaveRequested);
}

void MainWindow::updateUserDisplay()
{
    m_nicknameLabel->setText(m_nickname);
    m_avatarLabel->setPixmap(makeCircularPixmap(m_avatar, 32));
}

void MainWindow::onCreateMeetingClicked()
{
    CreateMeetingDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    // Switch to meeting window (mock — no real network yet)
    auto *stack = qobject_cast<QStackedWidget*>(centralWidget());
    if (stack) stack->setCurrentIndex(1);
}

void MainWindow::onJoinMeetingClicked()
{
    JoinMeetingDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString roomId = dlg.roomId();
    if (roomId.isEmpty()) {
        ConfirmDialog::showError("输入错误", "房间号不能为空。", this);
        return;
    }

    // Switch to meeting window (mock)
    auto *stack = qobject_cast<QStackedWidget*>(centralWidget());
    if (stack) stack->setCurrentIndex(1);
}

void MainWindow::onOpenSettingsClicked()
{
    UserProfileDialog dlg(this);
    dlg.setNickname(m_nickname);
    if (!m_avatar.isNull()) dlg.setAvatar(m_avatar);

    if (dlg.exec() == QDialog::Accepted) {
        QString nick = dlg.nickname();
        if (!nick.isEmpty()) m_nickname = nick;
        QPixmap av = dlg.avatar();
        if (!av.isNull()) m_avatar = av;
        updateUserDisplay();
    }
}

void MainWindow::onLeaveRequested()
{
    auto *stack = qobject_cast<QStackedWidget*>(centralWidget());
    if (stack) stack->setCurrentIndex(0);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_userContainer && event->type() == QEvent::MouseButtonRelease) {
        onOpenSettingsClicked();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}
