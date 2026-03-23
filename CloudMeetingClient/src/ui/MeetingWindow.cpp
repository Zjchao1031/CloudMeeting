/**
 * @file MeetingWindow.cpp
 * @brief 实现会议进行中的主界面窗口。
 */
#include "ui/MeetingWindow.h"
#include "ui/widgets/ParticipantListWidget.h"
#include "ui/widgets/ChatPanel.h"
#include "ui/widgets/VideoTileWidget.h"
#include "ui/widgets/ToolBarPanel.h"
#include "ui/dialogs/ConfirmDialog.h"
#include "app/AppContext.h"
#include "domain/service/ParticipantRepository.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QFrame>
#include <QTimer>

MeetingWindow::MeetingWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    bindSignals();
}

MeetingWindow::~MeetingWindow() {}

void MeetingWindow::setupUi()
{
    setWindowTitle("CloudMeeting - 会议中");
    resize(1280, 760);
    setMinimumSize(900, 600);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // 顶部栏。
    root->addWidget(makeTopBar());

    // 主内容分割区域。
    auto *mainSplit = new QSplitter(Qt::Horizontal, this);
    mainSplit->setHandleWidth(1);
    mainSplit->setStyleSheet("QSplitter::handle { background: #2E2E44; }");
    root->addWidget(mainSplit, 1);

    // 左侧视频滚动区域。
    auto *videoContainer = new QWidget(mainSplit);
    videoContainer->setStyleSheet("background: #121220;");
    auto *videoOuterLayout = new QVBoxLayout(videoContainer);
    videoOuterLayout->setContentsMargins(12, 12, 12, 12);
    videoOuterLayout->setSpacing(0);

    m_videoScrollArea = new QScrollArea(videoContainer);
    m_videoScrollArea->setWidgetResizable(true);
    m_videoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_videoScrollArea->setFrameShape(QFrame::NoFrame);
    m_videoScrollArea->setStyleSheet("background: transparent;");

    auto *videoInner = new QWidget(m_videoScrollArea);
    videoInner->setStyleSheet("background: transparent;");
    m_videoLayout = new QVBoxLayout(videoInner);
    m_videoLayout->setContentsMargins(0, 0, 0, 0);
    m_videoLayout->setSpacing(10);
    m_videoLayout->addStretch();

    m_videoScrollArea->setWidget(videoInner);
    videoOuterLayout->addWidget(m_videoScrollArea);
    mainSplit->addWidget(videoContainer);

    // 右侧参会者与聊天分割区域。
    auto *rightSplit = new QSplitter(Qt::Vertical, mainSplit);
    rightSplit->setHandleWidth(4);
    rightSplit->setStyleSheet(
        "QSplitter::handle { background: #2E2E44; }"
        "QSplitter::handle:hover { background: #4F8EF7; }"
    );
    rightSplit->setMinimumWidth(280);
    rightSplit->setMaximumWidth(380);

    m_participantList = new ParticipantListWidget(rightSplit);
    m_chatPanel       = new ChatPanel(rightSplit);

    rightSplit->addWidget(m_participantList);
    rightSplit->addWidget(m_chatPanel);
    rightSplit->setSizes({300, 400});
    mainSplit->addWidget(rightSplit);
    mainSplit->setSizes({900, 320});

    // 底部工具栏。
    m_toolBar = new ToolBarPanel(this);
    root->addWidget(m_toolBar);
}

QWidget* MeetingWindow::makeTopBar()
{
    auto *bar = new QWidget(this);
    bar->setFixedHeight(48);
    bar->setStyleSheet("background: #16162A; border-bottom: 1px solid #2E2E44;");

    auto *row = new QHBoxLayout(bar);
    row->setContentsMargins(16, 0, 16, 0);
    row->setSpacing(12);

    m_titleLabel = new QLabel("云会议", bar);
    m_titleLabel->setStyleSheet("color: #E8E8F0; font-size: 15px; font-weight: 700;");
    row->addWidget(m_titleLabel);
    row->addStretch();

    auto *roomHint = new QLabel("房间号：", bar);
    roomHint->setStyleSheet("color: #8888A8; font-size: 13px;");
    m_roomIdLabel = new QLabel("------", bar);
    m_roomIdLabel->setStyleSheet("color: #4F8EF7; font-size: 14px; font-weight: 700;");
    m_copyBtn = new QPushButton("复制", bar);
    m_copyBtn->setStyleSheet(
        "QPushButton { background: rgba(79,142,247,0.12); color: #4F8EF7;"
        "  border: 1px solid #4F8EF7; border-radius: 4px;"
        "  padding: 2px 8px; font-size: 12px; }"
        "QPushButton:hover { background: rgba(79,142,247,0.22); }"
    );
    m_copyBtn->setFixedHeight(24);
    row->addWidget(roomHint);
    row->addWidget(m_roomIdLabel);
    row->addWidget(m_copyBtn);
    row->addStretch();

    // 设备切换菜单。
    auto *deviceBtn = new QPushButton("设备", bar);
    deviceBtn->setStyleSheet(
        "QPushButton { background: #2A2A3E; color: #8888A8;"
        "  border: none; border-radius: 6px; padding: 4px 14px; font-size: 13px; }"
        "QPushButton:hover { color: #E8E8F0; background: #363650; }"
    );
    deviceBtn->setFixedHeight(32);
    auto *deviceMenu = new QMenu(bar);
    auto *camMenu    = deviceMenu->addMenu("切换摄像头");
    camMenu->addAction("内置摄像头 (默认)");
    camMenu->addAction("USB 摄像头")->setEnabled(false);
    auto *micMenu    = deviceMenu->addMenu("切换麦克风");
    micMenu->addAction("内置麦克风 (默认)");
    micMenu->addAction("耳机麦克风")->setEnabled(false);
    deviceBtn->setMenu(deviceMenu);
    row->addWidget(deviceBtn);

    return bar;
}

void MeetingWindow::bindSignals()
{
    // 复制房间号。
    connect(m_copyBtn, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(m_roomIdLabel->text());
        m_copyBtn->setText("已复制");
        QTimer::singleShot(1500, this, [this]() {
            m_copyBtn->setText("复制");
        });
    });

    // 退出会议确认。
    connect(m_toolBar, &ToolBarPanel::leaveRequested, this, [this]() {
        QString title, body, action;
        if (m_roomInfo.isHost) {
            title  = "关闭会议";
            body   = "您是会议主持人，退出将关闭整个会议并移除所有参会者，确认继续吗？";
            action = "关闭会议";
        } else {
            title  = "退出会议";
            body   = "确认退出当前会议吗？";
            action = "退出";
        }
        bool confirmed = ConfirmDialog::showDanger(title, body, action, this);
        if (confirmed) {
            emit leaveRequested();
        }
    });

    // 聊天面板发送消息 -> 向外转发信号。
    connect(m_chatPanel, &ChatPanel::messageSent, this, [this](const QString &text) {
        emit chatMessageSent(text);
    });

    // 工具栏音视频开关 -> 向外转发信号。
    connect(m_toolBar, &ToolBarPanel::cameraToggled, this, [this](bool on) {
        m_cameraOn = on;
        emit mediaStateChanged(m_cameraOn, m_micOn, m_screenShareOn);
    });
    connect(m_toolBar, &ToolBarPanel::micToggled, this, [this](bool on) {
        m_micOn = on;
        emit mediaStateChanged(m_cameraOn, m_micOn, m_screenShareOn);
    });
    connect(m_toolBar, &ToolBarPanel::screenShareToggled, this, [this](bool on) {
        m_screenShareOn = on;
        emit mediaStateChanged(m_cameraOn, m_micOn, m_screenShareOn);
    });
}

void MeetingWindow::setRoomInfo(const RoomInfo &room)
{
    m_roomInfo = room;
    m_roomIdLabel->setText(room.roomId);
    m_titleLabel->setText(room.isHost ? "云会议（主持人）" : "云会议");
}

void MeetingWindow::onParticipantsChanged()
{
    auto *repo = AppContext::instance().participantRepository();
    if (!repo) return;
    m_participantList->updateFromParticipants(repo->sortedParticipants());
}

void MeetingWindow::onNewChatMessage(const ChatMessage &msg)
{
    m_chatPanel->appendMessage(msg.userId, msg.nickname, msg.content);
}

void MeetingWindow::showRoomClosedDialog()
{
    ConfirmDialog::showInfo(
        "会议已结束",
        "主持人已关闭会议，您将返回主界面。",
        this
    );
    emit leaveRequested();
}
