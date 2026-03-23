#pragma once
#include "domain/model/RoomInfo.h"
#include "domain/model/ChatMessage.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSplitter>

class ParticipantListWidget;
class ChatPanel;
class VideoTileWidget;
class ToolBarPanel;

/**
 * @file MeetingWindow.h
 * @brief 声明会议进行中的主界面窗口。
 */

/**
 * @class MeetingWindow
 * @brief 展示视频区、参会者列表、聊天区和会议工具栏。
 */
class MeetingWindow : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造会议窗口。
     * @param[in] parent 父窗口指针。
     */
    explicit MeetingWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构会议窗口。
     */
    ~MeetingWindow();

    /**
     * @brief 设置当前房间信息并更新顶部栏显示。
     * @param[in] room 房间信息。
     */
    void setRoomInfo(const RoomInfo &room);

    /**
     * @brief 弹出会议已关闭提示并触发离开流程。
     */
    void showRoomClosedDialog();

public slots:
    /**
     * @brief 响应参会者列表数据变更，刷新列表 UI。
     */
    void onParticipantsChanged();

    /**
     * @brief 响应新聊天消息，追加到聊天面板。
     * @param[in] msg 新收到的聊天消息。
     */
    void onNewChatMessage(const ChatMessage &msg);

signals:
    /**
     * @brief 当用户确认离开会议时发出该信号。
     */
    void leaveRequested();

    /**
     * @brief 当用户发送聊天消息时发出该信号。
     * @param[in] text 消息文本。
     */
    void chatMessageSent(const QString &text);

    /**
     * @brief 当用户切换音视频开关时发出该信号。
     * @param[in] camera 摄像头开关状态。
     * @param[in] mic 麦克风开关状态。
     * @param[in] screen 屏幕共享开关状态。
     */
    void mediaStateChanged(bool camera, bool mic, bool screen);

private:
    /**
     * @brief 初始化会议窗口界面。
     */
    void setupUi();

    /**
     * @brief 绑定会议窗口内部组件的信号与槽。
     */
    void bindSignals();

    /**
     * @brief 创建会议窗口顶部栏。
     * @return 顶部栏控件指针。
     */
    QWidget* makeTopBar();

    QLabel                *m_titleLabel      = nullptr; ///< 顶部栏会议标题标签。
    QLabel                *m_roomIdLabel     = nullptr; ///< 顶部栏房间号标签。
    QPushButton           *m_copyBtn         = nullptr; ///< 复制房间号按钮。
    QScrollArea           *m_videoScrollArea = nullptr; ///< 视频区域滚动容器。
    QVBoxLayout           *m_videoLayout     = nullptr; ///< 视频区域布局管理器。
    ParticipantListWidget *m_participantList = nullptr; ///< 参会者列表组件。
    ChatPanel             *m_chatPanel       = nullptr; ///< 聊天面板组件。
    ToolBarPanel          *m_toolBar         = nullptr; ///< 底部工具栏组件。
    RoomInfo               m_roomInfo;                  ///< 当前房间信息缓存。
    bool                   m_cameraOn      = false;     ///< 本地摄像头状态。
    bool                   m_micOn         = false;     ///< 本地麦克风状态。
    bool                   m_screenShareOn = false;     ///< 本地屏幕共享状态。
};
