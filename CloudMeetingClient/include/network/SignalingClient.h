#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QTimer>
#include <QJsonObject>

class SignalingClient : public QObject
{
    Q_OBJECT
public:
    explicit SignalingClient(QObject *parent = nullptr);
    ~SignalingClient();

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    void sendPacket(quint8 type, const QJsonObject &payload);

signals:
    void packetReceived(quint8 type, QJsonObject payload);
    void connected();
    void disconnected();

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onHeartbeatTimeout();
    void handleReconnect();

private:
    void parsePackets();
    void startHeartbeat();

    QTcpSocket  m_socket;
    QByteArray  m_recvBuffer;
    QTimer      m_heartbeatTimer;
    QTimer      m_reconnectTimer;
    int         m_reconnectAttempts = 0;
    QString     m_host;
    quint16     m_port = 9000;
};
