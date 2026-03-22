/**
 * @file MediaUdpClient.cpp
 * @brief 实现 UDP 媒体传输客户端。
 */
#include "network/MediaUdpClient.h"

MediaUdpClient::MediaUdpClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_audioUdp, &QUdpSocket::readyRead, this, &MediaUdpClient::onAudioDatagram);
    connect(&m_videoUdp, &QUdpSocket::readyRead, this, &MediaUdpClient::onVideoDatagram);
}

MediaUdpClient::~MediaUdpClient() {}

void MediaUdpClient::init(const QString &serverHost, quint16 audioPort, quint16 videoPort)
{
    m_serverHost = serverHost;
    m_audioPort  = audioPort;
    m_videoPort  = videoPort;
    m_audioUdp.bind();
    m_videoUdp.bind();
}

void MediaUdpClient::sendAudioFrame(quint32 userId, const QByteArray &opusData)
{
    Q_UNUSED(userId)
    m_audioUdp.writeDatagram(opusData, QHostAddress(m_serverHost), m_audioPort);
}

void MediaUdpClient::sendVideoFrameFragments(quint32 userId, const QByteArray &h264Data, bool isCamera)
{
    Q_UNUSED(userId)
    Q_UNUSED(isCamera)
    // 待实现：补充分片发送逻辑。
    m_videoUdp.writeDatagram(h264Data, QHostAddress(m_serverHost), m_videoPort);
}

void MediaUdpClient::onAudioDatagram()
{
    while (m_audioUdp.hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_audioUdp.pendingDatagramSize()));
        m_audioUdp.readDatagram(data.data(), data.size());
        // 待实现：解析包头并提取 userId。
        emit audioDataReceived(0, data);
    }
}

void MediaUdpClient::onVideoDatagram()
{
    while (m_videoUdp.hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_videoUdp.pendingDatagramSize()));
        m_videoUdp.readDatagram(data.data(), data.size());
        // 待实现：解析包头并提取 userId 与视频来源标记。
        emit videoDataReceived(0, data, true);
    }
}
