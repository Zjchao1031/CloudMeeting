#include "network/NetworkFacade.h"
#include "network/SignalingClient.h"
#include "network/MediaUdpClient.h"

NetworkFacade::NetworkFacade()
    : m_signaling(new SignalingClient)
    , m_media(new MediaUdpClient)
{}

NetworkFacade::~NetworkFacade()
{
    delete m_signaling;
    delete m_media;
}

void NetworkFacade::connectToServer(const QString &host, quint16 port)
{
    m_signaling->connectToServer(host, port);
}

void NetworkFacade::disconnectFromServer()
{
    m_signaling->disconnectFromServer();
}

void NetworkFacade::sendCreateRoom(const QJsonObject &payload)
{
    m_signaling->sendPacket(0x01, payload);
}

void NetworkFacade::sendJoinRoom(const QJsonObject &payload)
{
    m_signaling->sendPacket(0x03, payload);
}

void NetworkFacade::sendLeaveRoom()
{
    m_signaling->sendPacket(0x05, {});
}

void NetworkFacade::sendMediaState(bool camera, bool mic, bool screen)
{
    QJsonObject obj;
    obj["camera"]       = camera;
    obj["microphone"]   = mic;
    obj["screen_share"] = screen;
    m_signaling->sendPacket(0x09, obj);
}

void NetworkFacade::sendChatMessage(const QString &text)
{
    QJsonObject obj;
    obj["content"] = text;
    m_signaling->sendPacket(0x0B, obj);
}
