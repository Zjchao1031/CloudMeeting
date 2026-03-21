#pragma once
#include <QString>
#include <QJsonObject>

class SignalingClient;
class MediaUdpClient;

class NetworkFacade
{
public:
    NetworkFacade();
    ~NetworkFacade();

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();

    void sendCreateRoom(const QJsonObject &payload);
    void sendJoinRoom(const QJsonObject &payload);
    void sendLeaveRoom();
    void sendMediaState(bool camera, bool mic, bool screen);
    void sendChatMessage(const QString &text);

private:
    SignalingClient  *m_signaling = nullptr;
    MediaUdpClient   *m_media     = nullptr;
};
