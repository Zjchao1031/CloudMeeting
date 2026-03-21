#include "network/PacketCodec.h"
#include <QJsonDocument>
#include <QDataStream>
#include <QIODevice>

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
