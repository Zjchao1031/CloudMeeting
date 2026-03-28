/**
 * @file SignalingClient.cpp
 * @brief 实现 TCP 信令客户端。
 */
#include "network/SignalingClient.h"
#include "network/PacketCodec.h"
#include "common/Constants.h"
#include "common/Logger.h"
#include "protocol/SignalType.h"
#include <QDataStream>
#include <QHostAddress>
#include <QNetworkProxy>

SignalingClient::SignalingClient(QObject *parent)
    : QObject(parent)
{
    m_socket.setProxy(QNetworkProxy::NoProxy);
    connect(&m_socket, &QTcpSocket::connected,    this, &SignalingClient::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &SignalingClient::onDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead,    this, &SignalingClient::onReadyRead);

    // [诊断] 监听 socket 错误信号。
    connect(&m_socket, &QAbstractSocket::errorOccurred, this, [this](QAbstractSocket::SocketError err) {
        Logger::error(QString("[SignalingClient] socket error: %1 (%2) state=%3")
                      .arg(static_cast<int>(err))
                      .arg(m_socket.errorString())
                      .arg(static_cast<int>(m_socket.state())));
    });

    // 周期性心跳发送定时器（每 5 秒）。
    m_heartbeatTimer.setInterval(Constants::HEARTBEAT_INTERVAL_MS);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &SignalingClient::onHeartbeatTimeout);

    // 心跳 ACK 超时检测（单次，10 秒）。
    m_heartbeatAckTimer.setSingleShot(true);
    m_heartbeatAckTimer.setInterval(Constants::HEARTBEAT_TIMEOUT_MS);
    connect(&m_heartbeatAckTimer, &QTimer::timeout, this, &SignalingClient::onHeartbeatAckTimeout);

    // 自动重连定时器（单次，2 秒后尝试）。
    m_reconnectTimer.setSingleShot(true);
    m_reconnectTimer.setInterval(Constants::RECONNECT_INTERVAL_MS);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &SignalingClient::handleReconnect);

    // 首次连接超时定时器（单次，5 秒）。
    m_connectTimer.setSingleShot(true);
    m_connectTimer.setInterval(Constants::CONNECT_TIMEOUT_MS);
    connect(&m_connectTimer, &QTimer::timeout, this, &SignalingClient::onConnectTimeout);
}

SignalingClient::~SignalingClient()
{
    m_connectTimer.stop();
    m_heartbeatTimer.stop();
    m_heartbeatAckTimer.stop();
    m_reconnectTimer.stop();
    m_socket.disconnectFromHost();
}

void SignalingClient::connectToServer(const QString &host, quint16 port)
{
    Logger::info(QString("[SignalingClient] connectToServer host=%1 port=%2 socketState=%3")
                .arg(host).arg(port).arg(static_cast<int>(m_socket.state())));
    m_host              = host;
    m_port              = port;
    m_reconnectAttempts = 0;
    m_wasConnected      = false;
    m_connectTimer.start();
    m_socket.connectToHost(host, port);
}

void SignalingClient::disconnectFromServer()
{
    m_connectTimer.stop();
    m_heartbeatTimer.stop();
    m_heartbeatAckTimer.stop();
    m_reconnectTimer.stop();
    m_wasConnected = false;
    m_socket.disconnectFromHost();
}

void SignalingClient::sendPacket(quint8 type, const QJsonObject &payload)
{
    m_socket.write(PacketCodec::encode(type, payload));
}

void SignalingClient::onConnected()
{
    Logger::info(QString("[SignalingClient] TCP connected to %1:%2").arg(m_host).arg(m_port));
    m_connectTimer.stop();
    bool wasReconnect   = m_wasConnected;
    m_wasConnected      = true;
    m_reconnectAttempts = 0;
    startHeartbeat();
    emit connected();
    if (wasReconnect) {
        emit reconnected();
    }
}

void SignalingClient::onDisconnected()
{
    m_heartbeatTimer.stop();
    m_heartbeatAckTimer.stop();
    emit disconnected();
    // 若连接过且重连次数未达上限，则安排重连。
    if (m_wasConnected && m_reconnectAttempts <= Constants::RECONNECT_MAX_ATTEMPTS) {
        m_reconnectTimer.start();
    }
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
        // 收到心跳 ACK 时重置超时检测定时器。
        if (type == static_cast<quint8>(SignalType::HEARTBEAT_ACK)) {
            m_heartbeatAckTimer.stop();
        }
        emit packetReceived(type, payload);
    }
}

void SignalingClient::startHeartbeat()
{
    m_heartbeatTimer.start();
}

void SignalingClient::onHeartbeatTimeout()
{
    // 发送心跳并启动 ACK 超时检测计时。
    sendPacket(static_cast<quint8>(SignalType::HEARTBEAT), {});
    m_heartbeatAckTimer.start();
}

void SignalingClient::onHeartbeatAckTimeout()
{
    // 10 秒内未收到 ACK，认为连接已断，主动断开并触发重连。
    m_heartbeatTimer.stop();
    m_socket.abort();
}

void SignalingClient::handleReconnect()
{
    ++m_reconnectAttempts;
    if (m_reconnectAttempts > Constants::RECONNECT_MAX_ATTEMPTS) {
        emit reconnectFailed();
        return;
    }
    m_socket.connectToHost(m_host, m_port);
}

void SignalingClient::onConnectTimeout()
{
    // 5 秒内未建立 TCP 连接，中止并通知上层。
    Logger::error(QString("[SignalingClient] connect timeout! host=%1 port=%2 socketState=%3 error=%4")
                 .arg(m_host).arg(m_port)
                 .arg(static_cast<int>(m_socket.state()))
                 .arg(m_socket.errorString()));
    m_socket.abort();
    emit connectTimeout();
}
