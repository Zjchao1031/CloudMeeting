/**
 * @file MediaUdpClient.cpp
 * @brief 实现 UDP 媒体传输客户端。
 */
#include "network/MediaUdpClient.h"
#include "network/PacketCodec.h"
#include "common/Constants.h"
#include <QHostAddress>
#include <QDateTime>

// 视频分片最大负载字节数（MTU 1500 - IP/UDP 头 - 视频包头）。
static constexpr int VIDEO_MTU_PAYLOAD = 1400;

// 视频包头 flags 位定义。
static constexpr quint8 FLAG_IDR        = 0x01; ///< IDR 关键帧标记。
static constexpr quint8 FLAG_FRAG_START = 0x02; ///< 分片起始标记。
static constexpr quint8 FLAG_FRAG_END   = 0x04; ///< 分片结束标记。

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
    // 绑定本地随机端口，用于接收下行数据。
    m_audioUdp.bind(QHostAddress::AnyIPv4, 0);
    m_videoUdp.bind(QHostAddress::AnyIPv4, 0);
}

void MediaUdpClient::sendAudioFrame(quint32 userId, const QByteArray &opusData)
{
    // 填充音频包头。
    AudioPacketHeader hdr;
    hdr.userId    = userId;
    hdr.timestamp = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFF);
    hdr.sequence  = m_audioSeq++;
    const QByteArray pkt = PacketCodec::encodeAudio(hdr, opusData);
    m_audioUdp.writeDatagram(pkt, QHostAddress(m_serverHost), m_audioPort);
}

void MediaUdpClient::sendVideoFrameFragments(quint32 userId, const QByteArray &h264Data, bool isCamera)
{
    const quint32 ts  = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFF);
    const int  total  = h264Data.size();
    const int  nFrags = (total + VIDEO_MTU_PAYLOAD - 1) / VIDEO_MTU_PAYLOAD;

    // 判断是否为 IDR 关键帧（NALU 类型 5 = IDR，起始码后第一个字节的低 5 位）。
    bool isIDR = false;
    if (total >= 5) {
        // 跳过 0x00 0x00 0x00 0x01 起始码。
        const int naluOffset = (h264Data[2] == 0x01) ? 3 : 4;
        if (naluOffset < total) {
            isIDR = (static_cast<quint8>(h264Data[naluOffset]) & 0x1F) == 5;
        }
    }

    for (int i = 0; i < nFrags; ++i) {
        const int offset = i * VIDEO_MTU_PAYLOAD;
        const int len    = qMin(VIDEO_MTU_PAYLOAD, total - offset);

        VideoPacketHeader hdr;
        hdr.userId    = userId;
        hdr.timestamp = ts;
        hdr.sequence  = m_videoSeq;
        hdr.flags     = 0;
        hdr.fragIndex = static_cast<quint8>(i);

        if (isIDR)              hdr.flags |= FLAG_IDR;
        if (i == 0)             hdr.flags |= FLAG_FRAG_START;
        if (i == nFrags - 1)    hdr.flags |= FLAG_FRAG_END;
        if (!isCamera)          hdr.flags |= 0x08; // bit3 标记屏幕共享来源。

        const QByteArray frag = h264Data.mid(offset, len);
        const QByteArray pkt  = PacketCodec::encodeVideo(hdr, frag);
        m_videoUdp.writeDatagram(pkt, QHostAddress(m_serverHost), m_videoPort);
    }
    ++m_videoSeq;
}

void MediaUdpClient::onAudioDatagram()
{
    while (m_audioUdp.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(m_audioUdp.pendingDatagramSize()));
        m_audioUdp.readDatagram(datagram.data(), datagram.size());

        AudioPacketHeader hdr;
        QByteArray opusData;
        if (!PacketCodec::decodeAudio(datagram, hdr, opusData)) continue;
        emit audioDataReceived(hdr.userId, opusData);
    }
}

void MediaUdpClient::onVideoDatagram()
{
    while (m_videoUdp.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(m_videoUdp.pendingDatagramSize()));
        m_videoUdp.readDatagram(datagram.data(), datagram.size());

        VideoPacketHeader hdr;
        QByteArray fragData;
        if (!PacketCodec::decodeVideo(datagram, hdr, fragData)) continue;

        const bool isCamera = !(hdr.flags & 0x08);
        const bool isStart  = hdr.flags & FLAG_FRAG_START;
        const bool isEnd    = hdr.flags & FLAG_FRAG_END;

        if (isStart && isEnd) {
            // 单包帧，无需重组，直接发出。
            emit videoDataReceived(hdr.userId, fragData, isCamera);
            continue;
        }

        // 分片重组：以 (userId, seq) 为 key 汇聚分片。
        const auto key = qMakePair(hdr.userId, hdr.sequence);
        auto &buf = m_videoFragBuf[key];
        buf.frags[hdr.fragIndex] = fragData;

        if (isEnd) {
            // 收到最后一片，记录总分片数。
            buf.totalFrags = hdr.fragIndex + 1;
        }

        // 若已收齐所有分片，则拼合并发出完整帧。
        if (buf.totalFrags > 0 && buf.frags.size() == static_cast<int>(buf.totalFrags)) {
            QByteArray frame;
            for (quint8 idx = 0; idx < buf.totalFrags; ++idx) {
                frame.append(buf.frags.value(idx));
            }
            m_videoFragBuf.remove(key);
            emit videoDataReceived(hdr.userId, frame, isCamera);
        }
    }
}
