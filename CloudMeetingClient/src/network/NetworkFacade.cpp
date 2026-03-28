/**
 * @file NetworkFacade.cpp
 * @brief 实现网络访问门面类。
 */
#include "network/NetworkFacade.h"
#include "network/SignalingClient.h"
#include "network/MediaUdpClient.h"
#include "protocol/SignalType.h"
#include "common/Constants.h"

NetworkFacade::NetworkFacade(QObject *parent)
    : QObject(parent)
    , m_signaling(new SignalingClient(this))
    , m_media(new MediaUdpClient(this))
{
    connect(m_signaling, &SignalingClient::connected,
            this,         &NetworkFacade::serverConnected);
    connect(m_signaling, &SignalingClient::disconnected,
            this,         &NetworkFacade::serverDisconnected);
    connect(m_signaling, &SignalingClient::reconnectFailed,
            this,         &NetworkFacade::serverReconnectFailed);
    connect(m_signaling, &SignalingClient::connectTimeout,
            this,         &NetworkFacade::serverConnectTimeout);
    connect(m_signaling, &SignalingClient::reconnected,
            this,         &NetworkFacade::onReconnected);
    connect(m_signaling, &SignalingClient::packetReceived,
            this,         &NetworkFacade::onPacketReceived);
}

NetworkFacade::~NetworkFacade() = default;

void NetworkFacade::connectToServer(const QString &host, quint16 port)
{
    m_signaling->connectToServer(host, port);
    // 初始化媒体 UDP 客户端（音频上行 9001，视频上行 9002）。
    m_media->init(host, Constants::UDP_AUDIO_UP_PORT, Constants::UDP_VIDEO_UP_PORT);
}

void NetworkFacade::disconnectFromServer()
{
    m_hasJoinCache = false;
    m_signaling->disconnectFromServer();
}

void NetworkFacade::sendCreateRoom(const QJsonObject &payload)
{
    m_signaling->sendPacket(static_cast<quint8>(SignalType::CREATE_ROOM), payload);
}

void NetworkFacade::sendJoinRoom(const QJsonObject &payload)
{
    // 缓存载荷，供断线重连后自动恢复会议使用。
    m_lastJoinPayload = payload;
    m_hasJoinCache    = true;
    m_signaling->sendPacket(static_cast<quint8>(SignalType::JOIN_ROOM), payload);
}

void NetworkFacade::sendLeaveRoom()
{
    m_hasJoinCache = false;
    m_signaling->sendPacket(static_cast<quint8>(SignalType::LEAVE_ROOM), {});
}

void NetworkFacade::sendMediaState(bool camera, bool mic, bool screen)
{
    QJsonObject obj;
    obj["camera"]       = camera;
    obj["microphone"]   = mic;
    obj["screen_share"] = screen;
    m_signaling->sendPacket(static_cast<quint8>(SignalType::MEDIA_STATE), obj);
}

void NetworkFacade::sendChatMessage(const QString &text)
{
    QJsonObject obj;
    obj["content"] = text;
    m_signaling->sendPacket(static_cast<quint8>(SignalType::CHAT_MESSAGE), obj);
}

void NetworkFacade::sendRequestKeyframe(const QString &targetUserId)
{
    QJsonObject obj;
    obj["target_user_id"] = targetUserId;
    m_signaling->sendPacket(static_cast<quint8>(SignalType::REQUEST_KEYFRAME), obj);
}

MediaUdpClient *NetworkFacade::mediaClient() const
{
    return m_media;
}

void NetworkFacade::onPacketReceived(quint8 type, QJsonObject payload)
{
    const auto t = static_cast<SignalType>(type);
    switch (t) {
    case SignalType::CREATE_ROOM_ACK: {
        const bool    ok  = payload["success"].toBool();
        const QString rid = payload["room_id"].toString();
        const QString uid = payload["user_id"].toString();
        const quint32 nid = payload["numeric_id"].toVariant().toUInt();
        const QString err = payload["error"].toString();
        emit createRoomAck(ok, rid, uid, nid, err);
        break;
    }
    case SignalType::JOIN_ROOM_ACK: {
        const bool    ok   = payload["success"].toBool();
        const QString rid  = payload["room_id"].toString();
        const QString host = payload["host_user_id"].toString();
        const QString uid  = payload["user_id"].toString();
        const quint32 nid  = payload["numeric_id"].toVariant().toUInt();
        const QString err  = payload["error"].toString();
        emit joinRoomAck(ok, rid, host, uid, nid, err);
        break;
    }
    case SignalType::ROOM_CLOSED:
        emit roomClosed();
        break;
    case SignalType::MEMBER_JOIN:
        emit memberJoined(payload);
        break;
    case SignalType::MEMBER_LEAVE:
        emit memberLeft(payload["user_id"].toString());
        break;
    case SignalType::MEDIA_STATE_SYNC:
        emit mediaStateSynced(payload);
        break;
    case SignalType::CHAT_BROADCAST:
        emit chatBroadcast(payload);
        break;
    case SignalType::REQUEST_KEYFRAME:
        emit keyframeRequested();
        break;
    default:
        break;
    }
}

void NetworkFacade::onReconnected()
{
    // 重连成功后自动重新发送 JOIN_ROOM 请求恢复会议状态。
    if (m_hasJoinCache) {
        m_signaling->sendPacket(static_cast<quint8>(SignalType::JOIN_ROOM), m_lastJoinPayload);
    }
}
