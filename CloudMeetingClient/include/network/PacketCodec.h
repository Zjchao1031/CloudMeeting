#pragma once
#include <QByteArray>
#include <QJsonObject>

class PacketCodec
{
public:
    // 编码：type(1) + length(4) + json payload(N)
    static QByteArray encode(quint8 type, const QJsonObject &payload);
    // 解码：从 buffer 中尝试解出一个完整包，成功返回 true 并填充 type/payload，消耗 consumed 字节
    static bool decode(const QByteArray &buffer, quint8 &type, QJsonObject &payload, int &consumed);
};
