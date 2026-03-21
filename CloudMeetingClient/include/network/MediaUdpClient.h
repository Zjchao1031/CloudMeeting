#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QByteArray>

class MediaUdpClient : public QObject
{
    Q_OBJECT
public:
    explicit MediaUdpClient(QObject *parent = nullptr);
    ~MediaUdpClient();

    void init(const QString &serverHost, quint16 audioPort, quint16 videoPort);
    void sendAudioFrame(quint32 userId, const QByteArray &opusData);
    void sendVideoFrameFragments(quint32 userId, const QByteArray &h264Data, bool isCamera);

signals:
    void audioDataReceived(quint32 userId, QByteArray data);
    void videoDataReceived(quint32 userId, QByteArray data, bool isCamera);

private slots:
    void onAudioDatagram();
    void onVideoDatagram();

private:
    QUdpSocket m_audioUdp;
    QUdpSocket m_videoUdp;
    QString    m_serverHost;
    quint16    m_audioPort = 9001;
    quint16    m_videoPort = 9002;
};
