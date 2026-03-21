#include "network/SignalingClient.h"
#include "network/PacketCodec.h"
#include "common/Constants.h"
#include <QDataStream>

SignalingClient::SignalingClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket, &QTcpSocket::connected,    this, &SignalingClient::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &SignalingClient::onDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead,    this, &SignalingClient::onReadyRead);

    m_heartbeatTimer.setInterval(Constants::HEARTBEAT_INTERVAL_MS);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &SignalingClient::onHeartbeatTimeout);

    m_reconnectTimer.setSingleShot(true);
    m_reconnectTimer.setInterval(Constants::RECONNECT_INTERVAL_MS);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &SignalingClient::handleReconnect);
}

SignalingClient::~SignalingClient()
{
    m_socket.disconnectFromHost();
}

void SignalingClient::connectToServer(const QString &host, quint16 port)
{
    m_host = host;
    m_port = port;
    m_reconnectAttempts = 0;
    m_socket.connectToHost(host, port);
}

void SignalingClient::disconnectFromServer()
{
    m_heartbeatTimer.stop();
    m_reconnectTimer.stop();
    m_socket.disconnectFromHost();
}

void SignalingClient::sendPacket(quint8 type, const QJsonObject &payload)
{
    m_socket.write(PacketCodec::encode(type, payload));
}

void SignalingClient::onConnected()
{
    m_reconnectAttempts = 0;
    startHeartbeat();
    emit connected();
}

void SignalingClient::onDisconnected()
{
    m_heartbeatTimer.stop();
    emit disconnected();
    if (m_reconnectAttempts < Constants::RECONNECT_MAX_ATTEMPTS)
        m_reconnectTimer.start();
}

void SignalingClient::onReadyRead()
{
    m_recvBuffer.append(m_socket.readAll());
    parsePackets();
}

void SignalingClient::parsePackets()
{
    while (true) {
        quint8      type;
        QJsonObject payload;
        int         consumed = 0;
        if (!PacketCodec::decode(m_recvBuffer, type, payload, consumed))
            break;
        m_recvBuffer.remove(0, consumed);
        emit packetReceived(type, payload);
    }
}

void SignalingClient::startHeartbeat()
{
    m_heartbeatTimer.start();
}

void SignalingClient::onHeartbeatTimeout()
{
    sendPacket(0x0D, {});
}

void SignalingClient::handleReconnect()
{
    m_reconnectAttempts++;
    m_socket.connectToHost(m_host, m_port);
}
