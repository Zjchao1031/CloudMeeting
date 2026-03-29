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
    // 定时器仅连接一次，由 init()/closeUdpSockets() 控制启停。
    connect(&m_gcTimer, &QTimer::timeout, this, &MediaUdpClient::onGcTimer);
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
    // 启动分片重组缓冲区定期清理，2 秒扫一次。
    m_gcTimer.start(2000);
}

void MediaUdpClient::closeUdpSockets()
{
    m_gcTimer.stop();
    m_audioUdp.close();
    m_videoUdp.close();
    m_videoFragBuf.clear();
    m_lastAudioSeq.clear();
    m_serverHost.clear();
    m_audioSeq = 0;
    m_videoSeq = 0;
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

        // 乱序包丢弃：quint16 无符号差运算天然支持序号绕回（RFC 3550）。
        if (m_lastAudioSeq.contains(hdr.userId)) {
            const quint16 diff = static_cast<quint16>(hdr.sequence - m_lastAudioSeq[hdr.userId]);
            // diff == 0：重复包；diff > 0x7FFF：落后超过半圈 → 均视为乱序，丢弃。
            if (diff == 0 || diff > 0x7FFF) continue;
        }
        m_lastAudioSeq[hdr.userId] = hdr.sequence;
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
        // 首片到达时记录时间戳，用于超时清理。
        if (buf.arrivedAt == 0)
            buf.arrivedAt = QDateTime::currentMSecsSinceEpoch();
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

void MediaUdpClient::sendUdpRegistration(quint32 numericId)
{
    // 音频注册包：仅包头，无 Opus 载荷。服务端收到后记录 udpAudioAddr。
    AudioPacketHeader audioHdr;
    audioHdr.userId    = numericId;
    audioHdr.timestamp = 0;
    audioHdr.sequence  = 0;
    const QByteArray audioPkt = PacketCodec::encodeAudio(audioHdr, QByteArray{});
    m_audioUdp.writeDatagram(audioPkt, QHostAddress(m_serverHost), m_audioPort);

    // 视频注册包：仅包头，无 H.264 载荷。服务端收到后记录 udpVideoAddr。
    VideoPacketHeader videoHdr;
    videoHdr.userId    = numericId;
    videoHdr.timestamp = 0;
    videoHdr.sequence  = 0;
    videoHdr.flags     = 0;
    videoHdr.fragIndex = 0;
    const QByteArray videoPkt = PacketCodec::encodeVideo(videoHdr, QByteArray{});
    m_videoUdp.writeDatagram(videoPkt, QHostAddress(m_serverHost), m_videoPort);
}

void MediaUdpClient::onGcTimer()
{
    constexpr qint64 TIMEOUT_MS  = 2000; ///< 分片重组超时阈值。
    constexpr int    MAX_ENTRIES = 128;  ///< 缓冲区条目硬上限。

    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    // 移除超过 2 秒仍未完成的分片条目。
    for (auto it = m_videoFragBuf.begin(); it != m_videoFragBuf.end(); ) {
        if (now - it->arrivedAt > TIMEOUT_MS)
            it = m_videoFragBuf.erase(it);
        else
            ++it;
    }

    // 硬上限兜底：超过 128 条时直接清空。
    if (m_videoFragBuf.size() > MAX_ENTRIES)
        m_videoFragBuf.clear();
}
