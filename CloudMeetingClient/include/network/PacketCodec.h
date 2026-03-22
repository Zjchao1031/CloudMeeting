#pragma once
#include <QByteArray>
#include <QJsonObject>

/**
 * @file PacketCodec.h
 * @brief 声明信令数据包编解码工具类。
 */

/**
 * @class PacketCodec
 * @brief 提供信令包的二进制编码与解码能力。
 */
class PacketCodec
{
public:
    /**
     * @brief 将消息类型与 JSON 载荷编码为二进制数据包。
     * @param[in] type 信令消息类型。
     * @param[in] payload 需要编码的 JSON 载荷。
     * @return 编码后的完整二进制数据包。
     * @note 数据格式为 `type(1) + length(4) + json payload(N)`。
     */
    static QByteArray encode(quint8 type, const QJsonObject &payload);

    /**
     * @brief 尝试从缓冲区中解码一个完整数据包。
     * @param[in] buffer 待解码的原始字节缓冲区。
     * @param[out] type 解码得到的消息类型。
     * @param[out] payload 解码得到的 JSON 载荷。
     * @param[out] consumed 本次解码成功后消耗的字节数。
     * @return 若成功解码出完整数据包则返回 `true`，否则返回 `false`。
     */
    static bool decode(const QByteArray &buffer, quint8 &type, QJsonObject &payload, int &consumed);
};
