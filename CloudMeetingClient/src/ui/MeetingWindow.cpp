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
    setWindowTitle("CloudMeeting - \u4f1a\u8bae\u4e2d");
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

    // 模拟视频卡片。
    auto *tile1 = new VideoTileWidget(videoInner);
    tile1->setWatermark("张三（主持人）");
    auto *tile2 = new VideoTileWidget(videoInner);
    tile2->setWatermark("李四");
    auto *tile3 = new VideoTileWidget(videoInner);
    tile3->setWatermark("王五");

    m_videoLayout->addWidget(tile1);
    m_videoLayout->addWidget(tile2);
    m_videoLayout->addWidget(tile3);
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

/**
 * @brief 创建顶部栏。
 * @return 顶部栏。
 */
QWidget* MeetingWindow::makeTopBar()
{
    auto *bar = new QWidget(this);
    bar->setFixedHeight(48);
    bar->setStyleSheet("background: #16162A; border-bottom: 1px solid #2E2E44;");

    auto *row = new QHBoxLayout(bar);
    row->setContentsMargins(16, 0, 16, 0);
    row->setSpacing(12);

    m_titleLabel = new QLabel("\u4e91\u4f1a\u8bae", bar);
    m_titleLabel->setStyleSheet("color: #E8E8F0; font-size: 15px; font-weight: 700;");
    row->addWidget(m_titleLabel);
    row->addStretch();

    auto *roomHint = new QLabel("\u623f\u95f4\u53f7\uff1a", bar);
    roomHint->setStyleSheet("color: #8888A8; font-size: 13px;");
    m_roomIdLabel = new QLabel("482910", bar);
    m_roomIdLabel->setStyleSheet("color: #4F8EF7; font-size: 14px; font-weight: 700;");
    m_copyBtn = new QPushButton("\u590d\u5236", bar);
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
    auto *camMenu    = deviceMenu->addMenu("\u5207\u6362\u6444\u50cf\u5934");
    camMenu->addAction("\u5185\u7f6e\u6444\u50cf\u5934 (\u9ed8\u8ba4)");
    camMenu->addAction("USB \u6444\u50cf\u5934")->setEnabled(false);
    auto *micMenu    = deviceMenu->addMenu("\u5207\u6362\u9ea6\u514b\u98ce");
    micMenu->addAction("\u5185\u7f6e\u9ea6\u514b\u98ce (\u9ed8\u8ba4)");
    micMenu->addAction("\u8033\u673a\u9ea6\u514b\u98ce")->setEnabled(false);
    deviceBtn->setMenu(deviceMenu);
    row->addWidget(deviceBtn);

    return bar;
}

void MeetingWindow::bindSignals()
{
    connect(m_copyBtn, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(m_roomIdLabel->text());
        m_copyBtn->setText("\u5df2\u590d\u5236");
        QTimer::singleShot(1500, this, [this]() {
            m_copyBtn->setText("\u590d\u5236");
        });
    });

    connect(m_toolBar, &ToolBarPanel::leaveRequested, this, [this]() {
        bool confirmed = ConfirmDialog::showDanger(
            "\u9000\u51fa\u4f1a\u8bae",
            "\u60a8\u662f\u4f1a\u8bae\u4e3b\u6301\u4eba\uff0c\u9000\u51fa\u5c06\u5173\u95ed\u6574\u4e2a\u4f1a\u8bae\u5e76\u79fb\u9664\u6240\u6709\u53c2\u4f1a\u8005\uff0c\u786e\u8ba4\u7ee7\u7eed\u5417\uff1f",
            "\u5173\u95ed\u4f1a\u8bae",
            this
        );
        if (confirmed) {
            emit leaveRequested();
        }
    });
}

void MeetingWindow::refreshLayout()
{
    m_participantList->refresh();
}

void MeetingWindow::showRoomClosedDialog()
{
    ConfirmDialog::showInfo(
        "\u4f1a\u8bae\u5df2\u7ed3\u675f",
        "\u4e3b\u6301\u4eba\u5df2\u5173\u95ed\u4f1a\u8bae\uff0c\u60a8\u5c06\u8fd4\u56de\u4e3b\u754c\u9762\u3002",
        this
    );
    emit leaveRequested();
}
