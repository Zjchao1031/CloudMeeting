#pragma once
#include "domain/model/RoomInfo.h"
#include "domain/model/ChatMessage.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSplitter>
#include <QHash>
#include <QSet>
#include <QString>
#include <QImage>
#include <QJsonObject>

class ParticipantListWidget;
class ChatPanel;
class VideoTileWidget;
class ToolBarPanel;
class QMenu;

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

    /**
     * @brief 接收远端视频帧，按需创建或刷新对应用户的视频 Tile。
     * @param[in] userId 视频帧所属用户 ID。
     * @param[in] frame  解码后的视频图像。
     * @param[in] isCamera 是否为摄像头流（否则为屏幕共享流）。
     */
    void onRemoteVideoFrame(const QString &userId, const QImage &frame, bool isCamera);

    /**
     * @brief 接收本地预览帧，摄像头和屏幕共享各渲染到独立 Tile。
     * @param[in] frame 本地采集的视频图像。
     * @param[in] isCamera 是否为摄像头流（否则为屏幕共享流）。
     */
    void onLocalVideoFrame(const QImage &frame, bool isCamera);

    /**
     * @brief 成员离开时移除其视频 Tile 并释放相关资源。
     * @param[in] userId 离开成员的用户 ID。
     */
    void onUserLeft(const QString &userId);

    /**
     * @brief 收到远端媒体状态同步通知时，移除已关闭流的视频 Tile，防止帧残留。
     * @param[in] payload 包含媒体状态信息的 JSON 载荷。
     */
    void onRemoteMediaStateSynced(QJsonObject payload);

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

    /**
     * @brief 当采集音量滑块变动时发出该信号。
     * @param[in] value 新的音量値（0≤10≤0）。
     */
    void captureVolumeChanged(int value);

    /**
     * @brief 当播放音量滑块变动时发出该信号。
     * @param[in] value 新的音量値（0≤10≤0）。
     */
    void playbackVolumeChanged(int value);

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
     * @brief 初始化会议窗口顶部栏。
     * @return 顶部栏控件指针。
     */
    QWidget* makeTopBar();

    /**
     * @brief 依据 DeviceManager 重新构建设备切换菜单项。
     */
    void refreshDeviceMenus();

    /**
     * @brief 处理单路视频帧渲染，包含 24fps 节流和 tile 创建。
     * @param[in] key   渲染用 key（远端 userId 或 "__local__"）。
     * @param[in] frame 待显示的视频图像。
     * @param[in] label Tile 上显示的水印文字。
     * @param[in] atTop 是否将 Tile 插入到列表顶部。
     */
    void renderToTile(const QString &key, const QImage &frame,
                      const QString &label, bool atTop);

    /**
     * @brief 移除指定 key 对应的视频 Tile 并释放相关资源。
     * @param[in] key Tile 的唯一标识。
     */
    void removeTile(const QString &key);

    QLabel                *m_titleLabel      = nullptr; ///< 顶部栏会议标题标签。
    QLabel                *m_roomIdLabel     = nullptr; ///< 顶部栏房间号标签。
    QPushButton           *m_copyBtn         = nullptr; ///< 复制房间号按钞。
    QScrollArea           *m_videoScrollArea = nullptr; ///< 视频区域滚动容器。
    QVBoxLayout           *m_videoLayout     = nullptr; ///< 视频区域布局管理器。
    ParticipantListWidget *m_participantList = nullptr; ///< 参会者列表组件。
    ChatPanel             *m_chatPanel       = nullptr; ///< 聊天面板组件。
    ToolBarPanel          *m_toolBar         = nullptr; ///< 底部工具栏组件。
    QMenu                 *m_camMenu         = nullptr; ///< 摄像头切换子菜单（用于动态刷新）。
    QMenu                 *m_micMenu         = nullptr; ///< 麦克风切换子菜单（用于动态刷新）。
    RoomInfo               m_roomInfo;                  ///< 当前房间信息缓存。
    bool                   m_cameraOn      = false;     ///< 本地摄像头状态。
    bool                   m_micOn         = false;     ///< 本地麦克风状态。
    bool                   m_screenShareOn = false;     ///< 本地屏幕共享状态。

    // 视频 Tile 动态管理。
    QHash<QString, VideoTileWidget*> m_videoTiles;         ///< 用户 ID → 视频卡片控件映射表。
    QHash<QString, qint64>           m_lastFrameTime;    ///< 用户 ID → 上次渲染时间戳（用于 24fps 节流）。
    QHash<QString, QString>          m_userNicknames;    ///< 用户 ID → 显示昵称映射表（UUID 与 numericId 字符串均作为键）。
    QHash<QString, quint32>          m_userNumericIds;   ///< UUID → numericId 映射表，用于将 TCP 信令中的 UUID 转换为 Tile key 所用的数字字符串。
    QSet<QString>                    m_suppressedTileKeys; ///< 已关闭的远端流 key 集合，阻断 UDP 残包重建 Tile。
};
