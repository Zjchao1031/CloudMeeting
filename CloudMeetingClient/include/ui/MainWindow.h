#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>

/**
 * @file MainWindow.h
 * @brief 声明客户端主窗口。
 */

class MeetingWindow;
class MeetingController;
class UserProfileService;
class ChatService;
class ParticipantRepository;
struct RoomInfo;

/**
 * @class MainWindow
 * @brief 提供会议创建、加入与用户资料入口的主界面窗口。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief 构造主窗口。
     * @param[in] parent 父窗口指针。
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构主窗口。
     */
    ~MainWindow();

protected:
    /**
     * @brief 处理主窗口内目标对象的事件过滤。
     * @param[in] obj 被监视的对象。
     * @param[in] event 当前分发的事件对象。
     * @return 若事件已被处理则返回 `true`，否则返回 `false`。
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    /**
     * @brief 响应创建会议按钮点击事件。
     */
    void onCreateMeetingClicked();

    /**
     * @brief 响应加入会议按钮点击事件。
     */
    void onJoinMeetingClicked();

    /**
     * @brief 打开用户设置对话框。
     */
    void onOpenSettingsClicked();

    /**
     * @brief 处理成功进入会议后的界面切换。
     * @param[in] room 当前会议房间信息。
     */
    void onMeetingEntered(const RoomInfo &room);

    /**
     * @brief 处理离开会议后的界面切换。
     */
    void onMeetingExited();

    /**
     * @brief 处理会议被主持人关闭的通知。
     */
    void onRoomClosed();

    /**
     * @brief 处理业务层错误提示。
     * @param[in] title 错误标题。
     * @param[in] message 错误详情。
     */
    void onErrorOccurred(const QString &title, const QString &message);

private:
    /**
     * @brief 初始化主窗口界面。
     */
    void setupUi();

    /**
     * @brief 绑定业务层信号与 UI 槽。
     */
    void bindServices();

    /**
     * @brief 根据当前用户资料刷新头像与昵称显示。
     */
    void updateUserDisplay();

    QWidget               *m_userContainer = nullptr; ///< 顶部用户信息点击区域。
    QLabel                *m_avatarLabel   = nullptr; ///< 顶部显示用户头像的标签。
    QLabel                *m_nicknameLabel = nullptr; ///< 顶部显示用户昵称的标签。
    MeetingWindow         *m_meetingWindow = nullptr; ///< 会议页面窗口组件。
    MeetingController     *m_meetingCtrl   = nullptr; ///< 会议控制器。
    UserProfileService    *m_profileSvc    = nullptr; ///< 用户资料服务。
    ChatService           *m_chatSvc       = nullptr; ///< 聊天服务。
    ParticipantRepository *m_participantRepo = nullptr; ///< 参会者仓库。
    bool m_localCameraOn      = false; ///< 本地摄像头当前是否开启。
    bool m_localScreenShareOn = false; ///< 本地屏幕共享当前是否开启。
};
