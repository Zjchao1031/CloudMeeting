#pragma once
#include <QByteArray>

/**
 * @file AudioDecoder.h
 * @brief 声明 Opus 音频解码器封装类。
 */

/**
 * @class AudioDecoder
 * @brief 封装音频解码器的打开、关闭与数据解码流程。
 */
class AudioDecoder
{
public:
    /**
     * @brief 构造音频解码器。
     */
    AudioDecoder();

    /**
     * @brief 析构音频解码器并释放资源。
     */
    ~AudioDecoder();

    /**
     * @brief 打开音频解码器。
     * @param[in] sampleRate 目标音频采样率。
     * @param[in] channels 目标音频通道数。
     * @return 若解码器成功初始化则返回 `true`，否则返回 `false`。
     */
    bool open(int sampleRate = 48000, int channels = 1);

    /**
     * @brief 关闭音频解码器。
     */
    void close();

    /**
     * @brief 将 Opus 编码数据解码为 PCM 原始音频数据。
     * @param[in] opusData 待解码的 Opus 音频帧。
     * @return 解码得到的 PCM 原始字节流。
     */
    QByteArray decode(const QByteArray &opusData);

private:
    bool m_opened = false; ///< 解码器是否已成功打开。
};
