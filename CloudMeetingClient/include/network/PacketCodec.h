#pragma once
#include <QByteArray>
#include <QJsonObject>
#include "protocol/MediaPacket.h"

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

    /**
     * @brief 将音频包头与 Opus 数据编码为 UDP 音频数据包。
     * @param[in] header 音频包头结构体。
     * @param[in] opusData Opus 编码后的音频数据。
     * @return 编码后的完整 UDP 音频数据包。
     */
    static QByteArray encodeAudio(const AudioPacketHeader &header, const QByteArray &opusData);

    /**
     * @brief 将视频包头与 H.264 分片数据编码为 UDP 视频数据包。
     * @param[in] header 视频包头结构体。
     * @param[in] h264Fragment H.264 分片数据。
     * @return 编码后的完整 UDP 视频数据包。
     */
    static QByteArray encodeVideo(const VideoPacketHeader &header, const QByteArray &h264Fragment);

    /**
     * @brief 从 UDP 音频数据包中解码包头与 Opus 数据。
     * @param[in] datagram 收到的完整 UDP 数据包。
     * @param[out] header 解码得到的音频包头。
     * @param[out] opusData 解码得到的 Opus 数据。
     * @return 解码成功返回 true，数据不足返回 false。
     */
    static bool decodeAudio(const QByteArray &datagram, AudioPacketHeader &header, QByteArray &opusData);

    /**
     * @brief 从 UDP 视频数据包中解码包头与 H.264 分片数据。
     * @param[in] datagram 收到的完整 UDP 数据包。
     * @param[out] header 解码得到的视频包头。
     * @param[out] h264Fragment 解码得到的 H.264 分片数据。
     * @return 解码成功返回 true，数据不足返回 false。
     */
    static bool decodeVideo(const QByteArray &datagram, VideoPacketHeader &header, QByteArray &h264Fragment);
};
