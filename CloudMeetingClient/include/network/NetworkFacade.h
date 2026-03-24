#pragma once
#include <QObject>
#include <QString>
#include <QJsonObject>

/**
 * @file NetworkFacade.h
 * @brief 声明网络访问门面类。
 */

class SignalingClient;
class MediaUdpClient;

/**
 * @class NetworkFacade
 * @brief 对外统一提供会议相关的网络发送能力，并将底层信令分发为业务层可用的信号。
 */
class NetworkFacade : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造网络门面。
     * @param[in] parent 父对象指针。
     */
    explicit NetworkFacade(QObject *parent = nullptr);

    /**
     * @brief 析构网络门面。
     */
    ~NetworkFacade() override;

    /**
     * @brief 连接到信令服务器。
     * @param[in] host 服务器主机地址。
     * @param[in] port 服务器端口。
     */
    void connectToServer(const QString &host, quint16 port);

    /**
     * @brief 断开与信令服务器的连接。
     */
    void disconnectFromServer();

    /**
     * @brief 发送创建会议请求。
     * @param[in] payload 创建会议所需的 JSON 载荷。
     */
    void sendCreateRoom(const QJsonObject &payload);

    /**
     * @brief 发送加入会议请求（同时缓存载荷，用于断线重连后自动恢复）。
     * @param[in] payload 加入会议所需的 JSON 载荷。
     */
    void sendJoinRoom(const QJsonObject &payload);

    /**
     * @brief 发送离开会议请求，并清除重连恢复缓存。
     */
    void sendLeaveRoom();

    /**
     * @brief 发送本地媒体状态同步消息。
     * @param[in] camera 摄像头开关状态。
     * @param[in] mic 麦克风开关状态。
     * @param[in] screen 屏幕共享开关状态。
     */
    void sendMediaState(bool camera, bool mic, bool screen);

    /**
     * @brief 发送聊天消息。
     * @param[in] text 待发送的聊天文本内容。
     */
    void sendChatMessage(const QString &text);

    /**
     * @brief 发送关键帧请求信令。
     * @param[in] targetUserId 需要重新发送关键帧的目标用户 ID。
     */
    void sendRequestKeyframe(const QString &targetUserId);

    /**
     * @brief 获取媒体 UDP 客户端。
     * @return 媒体 UDP 客户端指针。
     */
    MediaUdpClient *mediaClient() const;

signals:
    /**
     * @brief TCP 信令连接建立时发出。
     */
    void serverConnected();

    /**
     * @brief TCP 信令连接断开时发出。
     */
    void serverDisconnected();

    /**
     * @brief 多次重连失败时发出。
     */
    void serverReconnectFailed();

    /**
     * @brief 首次连接超时（5s 内未连上服务器）时发出。
     */
    void serverConnectTimeout();

    /**
     * @brief 收到创建会议响应时发出。
     * @param[in] success 是否创建成功。
     * @param[in] roomId 创建成功的房间号，失败时为空。
     * @param[in] errorMsg 失败原因，成功时为空。
     */
    void createRoomAck(bool success, const QString &roomId, const QString &errorMsg);

    /**
     * @brief 收到加入会议响应时发出。
     * @param[in] success 是否加入成功。
     * @param[in] roomId 加入的房间号，失败时为空。
     * @param[in] hostUserId 主持人用户 ID，失败时为空。
     * @param[in] errorMsg 失败原因，成功时为空。
     */
    void joinRoomAck(bool success, const QString &roomId,
                     const QString &hostUserId, const QString &errorMsg);

    /**
     * @brief 收到房间关闭通知时发出。
     */
    void roomClosed();

    /**
     * @brief 收到成员加入通知时发出。
     * @param[in] payload 包含新成员信息的 JSON 载荷。
     */
    void memberJoined(QJsonObject payload);

    /**
     * @brief 收到成员离开通知时发出。
     * @param[in] userId 离开成员的用户 ID。
     */
    void memberLeft(const QString &userId);

    /**
     * @brief 收到媒体状态同步通知时发出。
     * @param[in] payload 包含媒体状态信息的 JSON 载荷。
     */
    void mediaStateSynced(QJsonObject payload);

    /**
     * @brief 收到聊天消息广播时发出。
     * @param[in] payload 包含聊天消息信息的 JSON 载荷。
     */
    void chatBroadcast(QJsonObject payload);

private slots:
    /**
     * @brief 处理来自 SignalingClient 的原始数据包，分发为业务信号。
     * @param[in] type 信令消息类型字节。
     * @param[in] payload 信令消息 JSON 载荷。
     */
    void onPacketReceived(quint8 type, QJsonObject payload);

    /**
     * @brief 断线重连成功后自动重新加入会议。
     */
    void onReconnected();

private:
    SignalingClient  *m_signaling    = nullptr;  ///< 信令客户端对象。
    MediaUdpClient   *m_media        = nullptr;  ///< 媒体 UDP 客户端对象。
    QJsonObject       m_lastJoinPayload;         ///< 缓存最近一次 JOIN_ROOM 载荷，用于重连恢复。
    bool              m_hasJoinCache = false;    ///< 是否存在待恢复的 JOIN_ROOM 缓存。
};
