#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QTimer>
#include <QJsonObject>

/**
 * @file SignalingClient.h
 * @brief 声明 TCP 信令客户端。
 */

/**
 * @class SignalingClient
 * @brief 负责维护与信令服务器的 TCP 连接及消息收发。
 */
class SignalingClient : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造信令客户端。
     * @param[in] parent 父对象指针。
     */
    explicit SignalingClient(QObject *parent = nullptr);

    /**
     * @brief 析构信令客户端并断开当前连接。
     */
    ~SignalingClient();

    /**
     * @brief 连接到指定信令服务器。
     * @param[in] host 服务器主机地址。
     * @param[in] port 服务器端口。
     */
    void connectToServer(const QString &host, quint16 port);

    /**
     * @brief 主动断开与信令服务器的连接。
     */
    void disconnectFromServer();

    /**
     * @brief 发送一个信令数据包。
     * @param[in] type 信令消息类型。
     * @param[in] payload 消息对应的 JSON 载荷。
     */
    void sendPacket(quint8 type, const QJsonObject &payload);

signals:
    /**
     * @brief 当收到一个完整信令包时发出该信号。
     * @param[in] type 收到的消息类型。
     * @param[in] payload 收到的消息载荷。
     */
    void packetReceived(quint8 type, QJsonObject payload);

    /**
     * @brief 当 TCP 连接建立成功时发出该信号。
     */
    void connected();

    /**
     * @brief 当 TCP 连接断开时发出该信号。
     */
    void disconnected();

    /**
     * @brief 达到最大重连次数后仍未成功时发出该信号。
     */
    void reconnectFailed();

    /**
     * @brief 重连成功（即断线后再次建立连接）时发出该信号。
     */
    void reconnected();

    /**
     * @brief 首次连接超时（5s 内未建立 TCP 连接）时发出该信号。
     */
    void connectTimeout();

private slots:
    /**
     * @brief 处理套接字可读事件。
     */
    void onReadyRead();

    /**
     * @brief 处理连接建立成功事件。
     */
    void onConnected();

    /**
     * @brief 处理连接断开事件。
     */
    void onDisconnected();

    /**
     * @brief 处理心跳定时器超时事件（周期性发送 HEARTBEAT）。
     */
    void onHeartbeatTimeout();

    /**
     * @brief 处理心跳 ACK 超时事件（10s 未收到响应则触发重连）。
     */
    void onHeartbeatAckTimeout();

    /**
     * @brief 处理自动重连逻辑。
     */
    void handleReconnect();

    /**
     * @brief 处理首次连接超时事件。
     */
    void onConnectTimeout();

private:
    /**
     * @brief 解析接收缓冲区中的完整信令包。
     */
    void parsePackets();

    /**
     * @brief 启动心跳定时器。
     */
    void startHeartbeat();

    QTcpSocket  m_socket;                   ///< TCP 信令套接字。
    QTimer      m_connectTimer;              ///< 首次连接超时定时器（5s）。
    QByteArray  m_recvBuffer;               ///< 尚未完成解析的接收缓冲区。
    QTimer      m_heartbeatTimer;           ///< 定时发送心跳的定时器。
    QTimer      m_heartbeatAckTimer;        ///< 监测心跳 ACK 超时的定时器（10s）。
    QTimer      m_reconnectTimer;           ///< 自动重连使用的定时器。
    int         m_reconnectAttempts = 0;    ///< 当前已执行的重连次数。
    bool        m_wasConnected      = false;///< 标记是否曾经连接过（用于区分首次连接与重连）。
    QString     m_host;                     ///< 目标信令服务器主机地址。
    quint16     m_port = 9000;              ///< 目标信令服务器端口。
};
