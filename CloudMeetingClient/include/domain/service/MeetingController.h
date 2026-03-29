#pragma once
#include "domain/model/RoomInfo.h"
#include "domain/model/Participant.h"
#include <QObject>
#include <QString>
#include <QJsonObject>

/**
 * @file MeetingController.h
 * @brief 声明会议控制业务类及其参数结构。
 */

/**
 * @struct CreateRoomOptions
 * @brief 描述创建会议时需要提交的参数。
 */
struct CreateRoomOptions
{
    int     maxMembers  = 10;  ///< 会议允许的最大参会人数。
    bool    hasPassword = false; ///< 是否启用会议密码。
    QString password;          ///< 会议密码内容。
    QString nickname;          ///< 创建者昵称。
    QString avatarBase64;      ///< 创建者头像 Base64。
};

/**
 * @struct JoinRoomOptions
 * @brief 描述加入会议时需要提交的参数。
 */
struct JoinRoomOptions
{
    QString roomId;            ///< 目标会议房间号。
    QString password;          ///< 房间密码。
    QString nickname;          ///< 加入会议时使用的昵称。
    QString avatarBase64;      ///< 加入者头像 Base64。
};

class NetworkFacade;
class ParticipantRepository;

/**
 * @enum MeetingState
 * @brief 会议控制器状态机枚举。
 */
enum class MeetingState
{
    Idle,          ///< 空闲状态，未加入任何会议。
    Connecting,    ///< 正在连接服务器或等待响应。
    InMeeting,     ///< 已在会议中。
    Reconnecting   ///< 断线重连中。
};

/**
 * @class MeetingController
 * @brief 负责封装会议创建、加入和离开等业务流程，管理会议状态机。
 */
class MeetingController : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造会议控制器。
     * @param[in] parent 父对象指针。
     */
    explicit MeetingController(QObject *parent = nullptr);

    /**
     * @brief 设置底层网络门面对象。
     * @param[in] facade 负责实际网络通信的门面指针。
     */
    void setNetworkFacade(NetworkFacade *facade);

    /**
     * @brief 设置参会者仓库。
     * @param[in] repo 参会者数据仓库指针。
     */
    void setParticipantRepository(ParticipantRepository *repo);

    /**
     * @brief 设置服务器连接配置，替代默认硬编码地址。
     * @param[in] host 服务器主机地址。
     * @param[in] tcpPort 信令服务器 TCP 端口。
     * @param[in] udpAudioUpPort UDP 音频上行端口。
     * @param[in] udpVideoUpPort UDP 视频上行端口。
     */
    void setServerConfig(const QString &host, quint16 tcpPort,
                         quint16 udpAudioUpPort, quint16 udpVideoUpPort);

    /**
     * @brief 发起创建会议请求。
     * @param[in] opts 创建会议所需的参数集合。
     */
    void createRoom(const CreateRoomOptions &opts);

    /**
     * @brief 发起加入会议请求。
     * @param[in] opts 加入会议所需的参数集合。
     */
    void joinRoom(const JoinRoomOptions &opts);

    /**
     * @brief 发起离开会议操作。
     */
    void leaveRoom();

    /**
     * @brief 处理房间被关闭后的业务状态更新。
     */
    void onRoomClosed();

    /**
     * @brief 处理新成员加入事件。
     * @param[in] p 新加入的参会者信息。
     */
    void onMemberJoin(const Participant &p);

    /**
     * @brief 处理成员离开事件。
     * @param[in] userId 离开的成员用户标识。
     */
    void onMemberLeave(const QString &userId);

    /**
     * @brief 处理成员媒体状态同步事件。
     * @param[in] userId 目标成员用户标识。
     * @param[in] camera 摄像头开关状态。
     * @param[in] mic 麦克风开关状态。
     * @param[in] screen 屏幕共享开关状态。
     */
    void onMediaStateSync(const QString &userId, bool camera, bool mic, bool screen);

    /**
     * @brief 获取当前会议状态。
     * @return 当前状态机状态。
     */
    MeetingState state() const;

    /**
     * @brief 获取当前房间信息。
     * @return 当前房间信息的常量引用。
     */
    const RoomInfo &currentRoom() const;

    /**
     * @brief 获取服务器分配给本地用户的 UUID。
     * @return 本地用户 UUID 字符串；若尚未进入会议则返回空字符串。
     */
    QString localUserId() const;

    /**
     * @brief 获取服务器分配给本地用户的数字 ID（用于 UDP 媒体包头标识）。
     * @return 本地用户数字 ID；若尚未进入会议则返回 0。
     */
    quint32 localNumericId() const;

signals:
    /**
     * @brief 会议状态发生变更时发出该信号。
     * @param[in] newState 变更后的状态。
     */
    void stateChanged(MeetingState newState);

    /**
     * @brief 成功进入会议时发出该信号。
     * @param[in] room 当前会议房间信息。
     */
    void meetingEntered(const RoomInfo &room);

    /**
     * @brief 会议结束或离开时发出该信号。
     */
    void meetingExited();

    /**
     * @brief 会议被主持人关闭时发出该信号。
     */
    void roomClosed();

    /**
     * @brief 业务操作出错时发出该信号。
     * @param[in] title 错误标题。
     * @param[in] message 错误详情。
     */
    void errorOccurred(const QString &title, const QString &message);

private slots:
    /**
     * @brief 处理与信令服务器连接建立事件，向服务器发送已缓存的请求。
     */
    void onServerConnected();

    /**
     * @brief 处理最终重连失败事件。
     */
    void onServerReconnectFailed();

    /**
     * @brief 处理首次连接超时事件，回滚状态并提示用户。
     */
    void onServerConnectTimeout();

    /**
     * @brief 处理创建会议响应。
     * @param[in] success 是否创建成功。
     * @param[in] roomId  创建成功的房间号。
     * @param[in] userId  服务器分配的本地用户 UUID。
     * @param[in] numericId 服务器分配的本地用户数字 ID。
     * @param[in] errorMsg 失败原因。
     */
    void onCreateRoomAck(bool success, const QString &roomId,
                         const QString &userId, quint32 numericId,
                         const QString &errorMsg);

    /**
     * @brief 处理加入会议响应。
     * @param[in] success 是否加入成功。
     * @param[in] roomId 加入的房间号。
     * @param[in] hostUserId 主持人用户 ID。
     * @param[in] userId  服务器分配的本地用户 UUID。
     * @param[in] numericId 服务器分配的本地用户数字 ID。
     * @param[in] errorMsg 失败原因。
     */
    void onJoinRoomAck(bool success, const QString &roomId,
                       const QString &hostUserId,
                       const QString &userId, quint32 numericId,
                       const QString &errorMsg);

    /**
     * @brief 处理来自网络的成员加入通知。
     * @param[in] payload 包含成员信息的 JSON 载荷。
     */
    void onMemberJoinFromNetwork(QJsonObject payload);

    /**
     * @brief 处理来自网络的成员离开通知。
     * @param[in] userId 离开成员的用户 ID。
     */
    void onMemberLeaveFromNetwork(const QString &userId);

    /**
     * @brief 处理来自网络的媒体状态同步。
     * @param[in] payload 包含媒体状态的 JSON 载荷。
     */
    void onMediaStateSyncFromNetwork(QJsonObject payload);

    /**
     * @brief 处理来自网络的房间关闭通知。
     */
    void onRoomClosedFromNetwork();

private:
    /**
     * @brief 切换状态机状态。
     * @param[in] s 目标状态。
     */
    void setState(MeetingState s);

    MeetingState            m_state         = MeetingState::Idle; ///< 当前状态机状态。
    NetworkFacade          *m_network       = nullptr;            ///< 当前绑定的网络通信门面。
    ParticipantRepository  *m_repo          = nullptr;            ///< 参会者数据仓库。
    QString                 m_serverHost;                         ///< 信令服务器地址，从 profile.ini 注入。
    quint16                 m_serverTcpPort     = 0;              ///< 信令服务器 TCP 端口（从 profile.ini 读取）。
    quint16                 m_udpAudioUpPort    = 0;              ///< UDP 音频上行端口（从 profile.ini 读取）。
    quint16                 m_udpVideoUpPort    = 0;              ///< UDP 视频上行端口（从 profile.ini 读取）。
    RoomInfo                m_currentRoom;                        ///< 当前会议房间信息缓存。
    QString                 m_localUserId;                        ///< 当前用户在服务器分配的 UUID。
    quint32                 m_localNumericId = 0;                 ///< 当前用户在服务器分配的数字 ID（UDP 包头用）。

    // 缓存待发运作类型：0=无，1=创建，2=加入。
    int                     m_pendingAction = 0;
    CreateRoomOptions       m_pendingCreate;                ///< 缓存的创建会议参数。
    JoinRoomOptions         m_pendingJoin;                  ///< 缓存的加入会议参数。
};
