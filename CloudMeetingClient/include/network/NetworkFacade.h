#pragma once
#include <QString>
#include <QJsonObject>

/**
 * @file NetworkFacade.h
 * @brief 声明网络访问门面类。
 */

/**
 * @class NetworkFacade
 * @brief 对外统一提供会议相关的网络发送能力。
 */
class NetworkFacade
{
public:
    /**
     * @brief 构造网络门面。
     */
    NetworkFacade();

    /**
     * @brief 析构网络门面并释放底层网络对象。
     */
    ~NetworkFacade();

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
     * @brief 发送加入会议请求。
     * @param[in] payload 加入会议所需的 JSON 载荷。
     */
    void sendJoinRoom(const QJsonObject &payload);

    /**
     * @brief 发送离开会议请求。
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

private:
    SignalingClient  *m_signaling = nullptr; ///< 信令客户端对象。
    MediaUdpClient   *m_media     = nullptr; ///< 媒体 UDP 客户端对象。
};
