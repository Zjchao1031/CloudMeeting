/**
 * @file PacketCodec.cpp
 * @brief 实现信令数据包编解码工具类。
 */
#include "network/PacketCodec.h"
#include <QJsonDocument>
#include <QDataStream>
#include <QIODevice>
#include <cstring>

QByteArray PacketCodec::encode(quint8 type, const QJsonObject &payload)
{
    QByteArray json = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << type;
    ds << static_cast<quint32>(json.size());
    packet.append(json);
    return packet;
}

bool PacketCodec::decode(const QByteArray &buffer, quint8 &type, QJsonObject &payload, int &consumed)
{
    // 包头 = 1(type) + 4(length) = 5 字节
    if (buffer.size() < 5) return false;
    QDataStream ds(buffer);
    ds.setByteOrder(QDataStream::BigEndian);
    quint8  t;
    quint32 len;
    ds >> t >> len;
    if (buffer.size() < 5 + static_cast<int>(len)) return false;
    QByteArray json = buffer.mid(5, static_cast<int>(len));
    payload  = QJsonDocument::fromJson(json).object();
    type     = t;
    consumed = 5 + static_cast<int>(len);
    return true;
}

QByteArray PacketCodec::encodeAudio(const AudioPacketHeader &header, const QByteArray &opusData)
{
    // packet 初始为空，QDataStream 从偏移 0 开始追加。
    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << header.userId << header.timestamp << header.sequence;
    packet.append(opusData);
    return packet;
}

QByteArray PacketCodec::encodeVideo(const VideoPacketHeader &header, const QByteArray &h264Fragment)
{
    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << header.userId << header.timestamp << header.sequence << header.flags << header.fragIndex;
    packet.append(h264Fragment);
    return packet;
}

bool PacketCodec::decodeAudio(const QByteArray &datagram, AudioPacketHeader &header, QByteArray &opusData)
{
    constexpr int HDR_SIZE = static_cast<int>(sizeof(AudioPacketHeader)); // 10 字节
    if (datagram.size() < HDR_SIZE) return false;
    QDataStream ds(datagram);
    ds.setByteOrder(QDataStream::BigEndian);
    ds >> header.userId >> header.timestamp >> header.sequence;
    opusData = datagram.mid(HDR_SIZE);
    return true;
}

bool PacketCodec::decodeVideo(const QByteArray &datagram, VideoPacketHeader &header, QByteArray &h264Fragment)
{
    constexpr int HDR_SIZE = static_cast<int>(sizeof(VideoPacketHeader)); // 12 字节
    if (datagram.size() < HDR_SIZE) return false;
    QDataStream ds(datagram);
    ds.setByteOrder(QDataStream::BigEndian);
    ds >> header.userId >> header.timestamp >> header.sequence >> header.flags >> header.fragIndex;
    h264Fragment = datagram.mid(HDR_SIZE);
    return true;
}
