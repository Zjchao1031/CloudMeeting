#pragma once
#include <QByteArray>

struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwrContext;

/**
 * @file AudioEncoder.h
 * @brief 声明 Opus 音频编码器封装类。
 */

/**
 * @class AudioEncoder
 * @brief 封装音频编码器的打开、关闭与数据编码流程。
 */
class AudioEncoder
{
public:
    /**
     * @brief 构造音频编码器。
     */
    AudioEncoder();

    /**
     * @brief 析构音频编码器并释放资源。
     */
    ~AudioEncoder();

    /**
     * @brief 打开音频编码器。
     * @param[in] sampleRate 输入音频采样率。
     * @param[in] channels 输入音频通道数。
     * @return 若编码器成功初始化则返回 `true`，否则返回 `false`。
     */
    bool open(int sampleRate = 48000, int channels = 1);

    /**
     * @brief 关闭音频编码器。
     */
    void close();

    /**
     * @brief 将 PCM 原始音频数据编码为 Opus 帧。
     * @param[in] pcmData 待编码的 PCM 原始字节流（S16LE）。
     * @return 编码后的 Opus 音频帧数据。
     */
    QByteArray encode(const QByteArray &pcmData);

private:
    AVCodecContext *m_codecCtx = nullptr; ///< FFmpeg Opus 编码器上下文。
    SwrContext     *m_swrCtx   = nullptr; ///< 重采样上下文（输入→编码器所需格式）。
    AVFrame        *m_frame    = nullptr; ///< 编码输入帧缓冲区。
    AVPacket       *m_pkt      = nullptr; ///< 编码输出数据包。
    int             m_frameSize = 960;    ///< 每帧采样数（Opus 20ms@48kHz = 960）。
    int64_t         m_pts       = 0;      ///< 帧序号计数器。
    bool            m_opened    = false;  ///< 编码器是否已成功打开。
};
