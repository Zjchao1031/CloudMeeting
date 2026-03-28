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
#include "media/MediaEngine.h"
#include "media/device/DeviceManager.h"
#include "domain/service/UserProfileService.h"
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
#include <QDateTime>

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
    m_camMenu = deviceMenu->addMenu("切换摄像头");
    m_micMenu = deviceMenu->addMenu("切换麦克风");
    deviceBtn->setMenu(deviceMenu);
    row->addWidget(deviceBtn);

    // 使用 DeviceManager 填充菜单项。
    refreshDeviceMenus();

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

    // 工具栏音视频开关 -> 向外转发信号；关闭时同步移除本地 Tile 残帧。
    connect(m_toolBar, &ToolBarPanel::cameraToggled, this, [this](bool on) {
        m_cameraOn = on;
        if (!on) removeTile(QStringLiteral("__local_cam__"));
        emit mediaStateChanged(m_cameraOn, m_micOn, m_screenShareOn);
    });
    connect(m_toolBar, &ToolBarPanel::micToggled, this, [this](bool on) {
        m_micOn = on;
        emit mediaStateChanged(m_cameraOn, m_micOn, m_screenShareOn);
    });
    connect(m_toolBar, &ToolBarPanel::screenShareToggled, this, [this](bool on) {
        m_screenShareOn = on;
        if (!on) removeTile(QStringLiteral("__local_scr__"));
        emit mediaStateChanged(m_cameraOn, m_micOn, m_screenShareOn);
    });

    // 工具栏音量滑块 -> 向外转发信号。
    connect(m_toolBar, &ToolBarPanel::captureVolumeChanged,
            this,      &MeetingWindow::captureVolumeChanged);
    connect(m_toolBar, &ToolBarPanel::playbackVolumeChanged,
            this,      &MeetingWindow::playbackVolumeChanged);
}

void MeetingWindow::setRoomInfo(const RoomInfo &room)
{
    m_roomInfo = room;
    m_roomIdLabel->setText(room.roomId);
    m_titleLabel->setText(room.isHost ? "云会议（主持人）" : "云会议");
}

void MeetingWindow::refreshDeviceMenus()
{
    auto *dm = AppContext::instance().deviceManager();
    if (!dm || !m_camMenu || !m_micMenu) return;

    // 重建摄像头菜单项。
    m_camMenu->clear();
    const auto cams = dm->cameras();
    for (const auto &dev : cams) {
        QAction *act = m_camMenu->addAction(dev.name);
        act->setCheckable(true);
        act->setChecked(dev.id == dm->currentCameraId());
        connect(act, &QAction::triggered, this, [this, dev]() {
            auto *d  = AppContext::instance().deviceManager();
            auto *me = AppContext::instance().mediaEngine();
            if (d) d->switchCamera(dev.id);
            if (me && m_cameraOn) {
                me->stopCameraCapture();
                me->startCameraCapture(dev.id);
            }
            refreshDeviceMenus();
        });
    }
    if (cams.isEmpty())
        m_camMenu->addAction("（未检测到摄像头）")->setEnabled(false);

    // 重建麦克风菜单项。
    m_micMenu->clear();
    const auto mics = dm->microphones();
    for (const auto &dev : mics) {
        QAction *act = m_micMenu->addAction(dev.name);
        act->setCheckable(true);
        act->setChecked(dev.id == dm->currentMicId());
        connect(act, &QAction::triggered, this, [this, dev]() {
            auto *d  = AppContext::instance().deviceManager();
            auto *me = AppContext::instance().mediaEngine();
            if (d) d->switchMicrophone(dev.id);
            if (me && m_micOn) {
                me->stopAudioCapture();
                me->startAudioCapture(dev.id);
            }
            refreshDeviceMenus();
        });
    }
    if (mics.isEmpty())
        m_micMenu->addAction("（未检测到麦克风）")->setEnabled(false);
}

void MeetingWindow::renderToTile(const QString &key, const QImage &frame,
                                  const QString &label, bool atTop)
{
    // 24fps 节流：最小帧间隔约 41ms。
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastFrameTime.value(key, 0) < 41)
        return;
    m_lastFrameTime[key] = now;

    // 按需创建 VideoTileWidget。
    if (!m_videoTiles.contains(key)) {
        auto *tile = new VideoTileWidget(m_videoScrollArea->widget());
        tile->setWatermark(label);
        // 插入到布局：本地预览置顶，远端帧排在 stretch 之前。
        int insertPos = atTop ? 0 : m_videoLayout->count() - 1;
        m_videoLayout->insertWidget(insertPos, tile);
        m_videoTiles.insert(key, tile);
        tile->show();
    }
    m_videoTiles[key]->updateFrame(frame);
}

void MeetingWindow::removeTile(const QString &key)
{
    if (m_videoTiles.contains(key)) {
        VideoTileWidget *tile = m_videoTiles.take(key);
        m_videoLayout->removeWidget(tile);
        tile->deleteLater();
    }
    m_lastFrameTime.remove(key);
}

void MeetingWindow::onRemoteVideoFrame(const QString &userId, const QImage &frame, bool isCamera)
{
    const QString key = isCamera ? userId + QStringLiteral("_cam") : userId + QStringLiteral("_scr");
    // 流已关闭时丢弃 UDP 残包，防止残包重建 Tile 导致帧残留。
    if (m_suppressedTileKeys.contains(key)) return;
    const QString nickname = m_userNicknames.value(userId, userId);
    const QString label = isCamera ? nickname : nickname + QStringLiteral("（屏幕共享）");
    renderToTile(key, frame, label, false);
}

void MeetingWindow::onLocalVideoFrame(const QImage &frame, bool isCamera)
{
    // 流已关闭时丢弃残余帧，避免采集线程未完全退出前的帧重建 Tile。
    if (isCamera && !m_cameraOn)     return;
    if (!isCamera && !m_screenShareOn) return;

    auto *svc = AppContext::instance().userProfileService();
    const QString nickname = svc ? svc->nickname() : QStringLiteral("本地预览");
    const QString label = isCamera ? nickname + QStringLiteral("（本地）") : nickname + QStringLiteral("（屏幕共享）");
    const QString key   = isCamera ? QStringLiteral("__local_cam__") : QStringLiteral("__local_scr__");
    renderToTile(key, frame, label, true);
}

void MeetingWindow::onUserLeft(const QString &userId)
{
    // Tile key 以 numericId 字符串为准（与 UDP 帧路径一致）。
    const quint32 nid = m_userNumericIds.value(userId, 0);
    const QString tilePrefix = (nid != 0) ? QString::number(nid) : userId;

    // 同时移除摄像头流和屏幕共享流两个 Tile。
    removeTile(tilePrefix + QStringLiteral("_cam"));
    removeTile(tilePrefix + QStringLiteral("_scr"));
    m_suppressedTileKeys.remove(tilePrefix + QStringLiteral("_cam"));
    m_suppressedTileKeys.remove(tilePrefix + QStringLiteral("_scr"));
    m_userNicknames.remove(userId);
    m_userNicknames.remove(tilePrefix);
    m_userNumericIds.remove(userId);
}

void MeetingWindow::onRemoteMediaStateSynced(QJsonObject payload)
{
    const QString uid    = payload[QStringLiteral("user_id")].toString();
    const bool    camera = payload[QStringLiteral("camera")].toBool();
    const bool    screen = payload[QStringLiteral("screen_share")].toBool();
    if (uid.isEmpty()) return;

    // Tile key 必须与 UDP 帧路径保持一致：以 numericId 字符串为前缀。
    const quint32 nid = m_userNumericIds.value(uid, 0);
    const QString tilePrefix = (nid != 0) ? QString::number(nid) : uid;
    const QString camKey = tilePrefix + QStringLiteral("_cam");
    const QString scrKey = tilePrefix + QStringLiteral("_scr");

    // 流已关闭：加入压制集合并删除 Tile，阻断后续 UDP 残包重建 Tile。
    // 流重新开启：移出压制集合，恢复帧渲染。
    if (!camera) {
        m_suppressedTileKeys.insert(camKey);
        removeTile(camKey);
    } else {
        m_suppressedTileKeys.remove(camKey);
    }
    if (!screen) {
        m_suppressedTileKeys.insert(scrKey);
        removeTile(scrKey);
    } else {
        m_suppressedTileKeys.remove(scrKey);
    }
}

void MeetingWindow::onParticipantsChanged()
{
    auto *repo = AppContext::instance().participantRepository();
    if (!repo) return;

    const auto participants = repo->sortedParticipants();
    m_participantList->updateFromParticipants(participants);

    // 同步更新昵称映射（UUID 和 numericId 字符串均作为 key），并维护 UUID → numericId 映射表。
    for (const auto &p : participants) {
        m_userNicknames[p.userId] = p.nickname;
        if (p.numericId != 0) {
            m_userNicknames[QString::number(p.numericId)] = p.nickname;
            m_userNumericIds[p.userId] = p.numericId;
        }
    }

    for (auto it = m_videoTiles.begin(); it != m_videoTiles.end(); ++it) {
        const QString &key = it.key();
        // 提取原始 userId（去掉 _cam / _scr 后缀）。
        QString uid = key;
        if (uid.endsWith(QStringLiteral("_cam")))      uid.chop(4);
        else if (uid.endsWith(QStringLiteral("_scr"))) uid.chop(4);
        if (m_userNicknames.contains(uid)) {
            const bool isCamTile = key.endsWith(QStringLiteral("_cam"));
            const QString label  = isCamTile ? m_userNicknames[uid]
                                             : m_userNicknames[uid] + QStringLiteral("（屏幕共享）");
            it.value()->setWatermark(label);
        }
    }
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
