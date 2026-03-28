/**
 * @file MainWindow.cpp
 * @brief 实现客户端主窗口逻辑。
 */
#include "ui/MainWindow.h"
#include "ui/MeetingWindow.h"
#include "ui/dialogs/UserProfileDialog.h"
#include "ui/dialogs/CreateMeetingDialog.h"
#include "ui/dialogs/JoinMeetingDialog.h"
#include "ui/dialogs/ConfirmDialog.h"
#include "app/AppContext.h"
#include "domain/service/UserProfileService.h"
#include "domain/service/MeetingController.h"
#include "domain/service/ChatService.h"
#include "domain/service/ParticipantRepository.h"
#include "network/NetworkFacade.h"
#include "media/MediaEngine.h"
#include "media/device/DeviceManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QStackedWidget>
#include <QFrame>
#include <QApplication>
#include <QPolygon>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>

/**
 * @brief 将头像裁剪为圆形图像。
 * @param[in] src 原始头像图像。
 * @param[in] size 输出图像边长尺寸，单位：像素。
 * @return 裁剪后的圆形头像图像。
 */
static QPixmap makeCircularPixmap(const QPixmap &src, int size)
{
    if (src.isNull()) return QPixmap();
    QPixmap scaled = src.scaled(size, size,
        Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath clipPath;
    clipPath.addEllipse(QRectF(0.0, 0.0, size, size));
    p.setClipPath(clipPath);
    int ox = (scaled.width()  - size) / 2;
    int oy = (scaled.height() - size) / 2;
    p.drawPixmap(QRect(0, 0, size, size), scaled, QRect(ox, oy, size, size));
    return result;
}

/**
 * @brief 刷新控件动态属性变更后的 QSS 样式状态。
 * @param[in] widget 待刷新样式的目标控件。
 */
static void refreshWidgetStyle(QWidget *widget)
{
    if (widget == nullptr) return;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto &ctx = AppContext::instance();
    m_profileSvc      = ctx.userProfileService();
    m_meetingCtrl     = ctx.meetingController();
    m_chatSvc         = ctx.chatService();
    m_participantRepo = ctx.participantRepository();

    setupUi();
    bindServices();
    updateUserDisplay();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    setWindowTitle("CloudMeeting");
    resize(960, 640);
    setMinimumSize(800, 560);

    auto *stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // 首页内容页。
    auto *mainPage = new QWidget(stack);
    mainPage->setStyleSheet("background: #1E1E2E;");

    auto *pageLayout = new QVBoxLayout(mainPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // 顶部栏区域。
    auto *topBar = new QWidget(mainPage);
    topBar->setFixedHeight(60);
    topBar->setStyleSheet("background: #16162A; border-bottom: 1px solid #2A2A3E;");
    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 0, 20, 0);
    topBarLayout->setSpacing(0);

    // 左侧应用名称。
    auto *appNameLabel = new QLabel("CloudMeeting", topBar);
    appNameLabel->setStyleSheet(
        "color: #4F8EF7; font-size: 16px; font-weight: 700; letter-spacing: 1px;");
    topBarLayout->addWidget(appNameLabel);
    topBarLayout->addStretch();

    // 右侧用户头像与昵称区域，使用事件过滤处理点击。
    m_userContainer = new QWidget(topBar);
    m_userContainer->setObjectName("userContainer");
    m_userContainer->setAttribute(Qt::WA_StyledBackground, true);
    m_userContainer->setAttribute(Qt::WA_Hover, true);
    m_userContainer->setProperty("active", false);
    m_userContainer->setStyleSheet(
        "QWidget#userContainer {"
        " background-color: transparent;"
        " border: none;"
        " border-radius: 8px;"
        "}"
        "QWidget#userContainer:hover,"
        "QWidget#userContainer[active=\"true\"] {"
        " background-color: rgba(255,255,255,0.07);"
        "}"
        "QWidget#userContainer QLabel {"
        " background-color: transparent;"
        " border: none;"
        "}"
    );
    m_userContainer->setCursor(Qt::PointingHandCursor);
    m_userContainer->installEventFilter(this);

    auto *userBtnLayout = new QHBoxLayout(m_userContainer);
    userBtnLayout->setContentsMargins(8, 4, 10, 4);
    userBtnLayout->setSpacing(8);

    m_avatarLabel = new QLabel(m_userContainer);
    m_avatarLabel->setFixedSize(32, 32);
    m_avatarLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_avatarLabel->setStyleSheet("background: transparent;");

    m_nicknameLabel = new QLabel(m_userContainer);
    m_nicknameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_nicknameLabel->setStyleSheet("background: transparent; color: #E8E8F0; font-size: 14px;");

    auto *chevron = new QLabel("\u25be", m_userContainer);
    chevron->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    chevron->setStyleSheet("background: transparent; color: #8888A8; font-size: 11px;");

    userBtnLayout->addWidget(m_avatarLabel);
    userBtnLayout->addWidget(m_nicknameLabel);
    userBtnLayout->addWidget(chevron);

    topBarLayout->addWidget(m_userContainer);

    pageLayout->addWidget(topBar);

    // 中央主展示区域。
    auto *hero = new QWidget(mainPage);
    hero->setStyleSheet("background: transparent;");
    auto *heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(40, 0, 40, 40);
    heroLayout->setSpacing(0);
    heroLayout->setAlignment(Qt::AlignCenter);

    // 应用图标。
    QPixmap logoPix(80, 80);
    logoPix.fill(Qt::transparent);
    {
        QPainter lp(&logoPix);
        lp.setRenderHint(QPainter::Antialiasing);
        lp.setBrush(QColor("#4F8EF7"));
        lp.setPen(Qt::NoPen);
        lp.drawRoundedRect(0, 0, 80, 80, 20, 20);
        // 摄像机主体。
        lp.setBrush(Qt::white);
        lp.drawRoundedRect(8, 24, 40, 30, 6, 6);
        // 镜头部分。
        lp.setBrush(QColor("#4F8EF7"));
        lp.drawEllipse(17, 29, 20, 20);
        lp.setBrush(Qt::white);
        lp.drawEllipse(22, 34, 10, 10);
        // 取景器部分。
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

    // 产品名称与标语。
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

    // 主操作按钮区域。
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

    // 会议进行页面。
    m_meetingWindow = new MeetingWindow(stack);
    stack->addWidget(m_meetingWindow);  // index 1

}

void MainWindow::bindServices()
{
    // MeetingController 信号 -> MainWindow 槽。
    connect(m_meetingCtrl, &MeetingController::meetingEntered,
            this, &MainWindow::onMeetingEntered);
    connect(m_meetingCtrl, &MeetingController::meetingExited,
            this, &MainWindow::onMeetingExited);
    connect(m_meetingCtrl, &MeetingController::roomClosed,
            this, &MainWindow::onRoomClosed);
    connect(m_meetingCtrl, &MeetingController::errorOccurred,
            this, &MainWindow::onErrorOccurred);

    // MeetingWindow 离开请求 -> MeetingController::leaveRoom。
    connect(m_meetingWindow, &MeetingWindow::leaveRequested,
            this, [this]() { m_meetingCtrl->leaveRoom(); });

    // ParticipantRepository 变更 -> MeetingWindow 刷新参会者列表。
    connect(m_participantRepo, &ParticipantRepository::participantsChanged,
            m_meetingWindow, &MeetingWindow::onParticipantsChanged);

    // ChatService 新消息 -> MeetingWindow 追加聊天消息。
    connect(m_chatSvc, &ChatService::newMessageReceived,
            m_meetingWindow, &MeetingWindow::onNewChatMessage);

    // MeetingWindow 聊天发送 -> ChatService::sendMessage。
    connect(m_meetingWindow, &MeetingWindow::chatMessageSent,
            m_chatSvc, &ChatService::sendMessage);

    // MeetingWindow 媒体状态切换 -> NetworkFacade 发送状态 + MediaEngine 启停采集。
    connect(m_meetingWindow, &MeetingWindow::mediaStateChanged,
            this, [this](bool camera, bool mic, bool screen) {
                m_localCameraOn      = camera;
                m_localScreenShareOn = screen;

                auto *ne = AppContext::instance().networkFacade();
                if (ne) ne->sendMediaState(camera, mic, screen);

                // 本地仓库同步更新：服务器不会将 MEDIA_STATE_SYNC 回送给发送者，
                // 需手动更新本地用户在 ParticipantRepository 中的媒体状态，
                // 以确保参会者列表右侧图标能即时反映本地用户的切换操作。
                m_meetingCtrl->onMediaStateSync(
                    m_meetingCtrl->localUserId(), camera, mic, screen);

                auto *me = AppContext::instance().mediaEngine();
                auto *dm = AppContext::instance().deviceManager();
                if (!me) return;

                if (camera)
                    me->startCameraCapture(dm ? dm->currentCameraId() : QString{});
                else
                    me->stopCameraCapture();

                if (mic)
                    me->startAudioCapture(dm ? dm->currentMicId() : QString{});
                else
                    me->stopAudioCapture();

                if (screen)
                    me->startScreenShare();
                else
                    me->stopScreenShare();
            });

    // 音量滑块 -> MediaEngine 音量设置。
    connect(m_meetingWindow, &MeetingWindow::captureVolumeChanged,
            this, [](int value) {
                auto *me = AppContext::instance().mediaEngine();
                if (me) me->setCaptureVolume(value);
            });
    connect(m_meetingWindow, &MeetingWindow::playbackVolumeChanged,
            this, [](int value) {
                auto *me = AppContext::instance().mediaEngine();
                if (me) me->setPlaybackVolume(value);
            });

    // MediaEngine 远端/本地视频帧 -> MeetingWindow Tile 展示。
    auto *me = AppContext::instance().mediaEngine();
    if (me) {
        connect(me, &MediaEngine::remoteVideoFrame,
                m_meetingWindow, &MeetingWindow::onRemoteVideoFrame);
        connect(me, &MediaEngine::localVideoFrame,
                m_meetingWindow, &MeetingWindow::onLocalVideoFrame);
    }

    // 成员离开 -> MeetingWindow 移除 Tile。
    auto *nf = AppContext::instance().networkFacade();
    if (nf) {
        connect(nf, &NetworkFacade::memberLeft,
                m_meetingWindow, &MeetingWindow::onUserLeft);

        // 远端媒体状态变化 -> MeetingWindow 清除已关闭流的 Tile，防止帧残留。
        connect(nf, &NetworkFacade::mediaStateSynced,
                m_meetingWindow, &MeetingWindow::onRemoteMediaStateSynced);

        // 成员加入时的关键帧协商：
        // 1. 若新加入的成员已在广播（我们是后来者收到已有成员信息），向其请求关键帧。
        // 2. 若我方正在广播（新成员加入），强制输出关键帧让新成员能开始解码。
        connect(nf, &NetworkFacade::memberJoined, this, [this, nf](QJsonObject payload) {
            const bool remoteCamera = payload[QStringLiteral("camera")].toBool();
            const bool remoteScreen = payload[QStringLiteral("screen_share")].toBool();
            const QString uid = payload[QStringLiteral("user_id")].toString();

            if ((remoteCamera || remoteScreen) && !uid.isEmpty())
                nf->sendRequestKeyframe(uid);

            if (m_localCameraOn || m_localScreenShareOn) {
                auto *me = AppContext::instance().mediaEngine();
                if (me) me->forceVideoKeyFrame();
            }
        });
    }
}

void MainWindow::updateUserDisplay()
{
    if (!m_profileSvc) return;
    m_nicknameLabel->setText(m_profileSvc->nickname());
    QImage avatarImg = m_profileSvc->avatar();
    if (!avatarImg.isNull()) {
        m_avatarLabel->setPixmap(makeCircularPixmap(
            QPixmap::fromImage(avatarImg), 32));
    }
}

void MainWindow::onCreateMeetingClicked()
{
    CreateMeetingDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    CreateRoomOptions opts;
    opts.maxMembers  = dlg.maxMembers();
    opts.hasPassword = dlg.hasPassword();
    opts.password    = dlg.password();
    opts.nickname    = m_profileSvc->nickname();
    opts.avatarBase64 = m_profileSvc->avatarBase64();

    m_meetingCtrl->createRoom(opts);
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

    JoinRoomOptions opts;
    opts.roomId      = roomId;
    opts.password    = dlg.password();
    opts.nickname    = m_profileSvc->nickname();
    opts.avatarBase64 = m_profileSvc->avatarBase64();

    m_meetingCtrl->joinRoom(opts);
}

void MainWindow::onOpenSettingsClicked()
{
    UserProfileDialog dlg(this);
    dlg.setNickname(m_profileSvc->nickname());
    QImage avatarImg = m_profileSvc->avatar();
    if (!avatarImg.isNull()) {
        dlg.setAvatar(QPixmap::fromImage(avatarImg));
    }

    if (dlg.exec() == QDialog::Accepted) {
        QString nick = dlg.nickname();
        if (!nick.isEmpty()) {
            m_profileSvc->setNickname(nick);
        }
        QPixmap av = dlg.avatar();
        if (!av.isNull()) {
            m_profileSvc->setAvatar(av.toImage());
        }
        m_profileSvc->save();
        updateUserDisplay();
    }
}

void MainWindow::onMeetingEntered(const RoomInfo &room)
{
    // 通知 MeetingWindow 设置房间信息。
    m_meetingWindow->setRoomInfo(room);

    // 切换到会议页面。
    auto *stack = qobject_cast<QStackedWidget*>(centralWidget());
    if (stack) stack->setCurrentIndex(1);
}

void MainWindow::onMeetingExited()
{
    // 清空聊天缓存。
    m_chatSvc->clearMessages();

    // 切换回主界面。
    auto *stack = qobject_cast<QStackedWidget*>(centralWidget());
    if (stack) stack->setCurrentIndex(0);
}

void MainWindow::onRoomClosed()
{
    m_chatSvc->clearMessages();
    m_meetingWindow->showRoomClosedDialog();
    // onRoomClosedFromNetwork() 已将状态置为 Idle，leaveRoom() 会提前返回不再 emit meetingExited()，
    // 需在此直接切换回主界面，不依赖 leaveRequested → leaveRoom → meetingExited 链路。
    auto *stack = qobject_cast<QStackedWidget*>(centralWidget());
    if (stack) stack->setCurrentIndex(0);
}

void MainWindow::onErrorOccurred(const QString &title, const QString &message)
{
    ConfirmDialog::showError(title, message, this);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_userContainer) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_userContainer->setProperty("active", true);
                refreshWidgetStyle(m_userContainer);
                return true;
            }
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            auto *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_userContainer->setProperty("active", true);
                refreshWidgetStyle(m_userContainer);
                onOpenSettingsClicked();
                m_userContainer->setProperty("active", false);
                refreshWidgetStyle(m_userContainer);
                return true;
            }
        }

        if (event->type() == QEvent::Leave && m_userContainer->property("active").toBool()) {
            m_userContainer->setProperty("active", false);
            refreshWidgetStyle(m_userContainer);
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
