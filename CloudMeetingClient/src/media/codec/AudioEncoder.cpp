/**
 * @file AudioEncoder.cpp
 * @brief 实现 Opus 音频编码器封装类。
 */
#include "media/codec/AudioEncoder.h"
#include "common/Logger.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

AudioEncoder::AudioEncoder() {}

AudioEncoder::~AudioEncoder() { close(); }

bool AudioEncoder::open(int sampleRate, int channels)
{
    if (m_opened) return true;

    // 优先 libopus，其次内置 opus。
    const AVCodec *codec = avcodec_find_encoder_by_name("libopus");
    if (!codec) codec = avcodec_find_encoder(AV_CODEC_ID_OPUS);
    if (!codec) {
        Logger::warn(QStringLiteral("[AudioEncoder] 未找到 Opus 编码器"));
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    m_codecCtx->sample_rate = sampleRate;
    m_codecCtx->bit_rate    = 64000; // 64 kbps，语音场景足够。
    m_codecCtx->sample_fmt  = AV_SAMPLE_FMT_S16; // libopus 要求 S16 或 FLT。
    av_channel_layout_default(&m_codecCtx->ch_layout, channels);

    // 尝试 S16 格式打开，如果编码器不支持则尝试 FLT。
    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        m_codecCtx->sample_fmt = AV_SAMPLE_FMT_FLT;
        if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
            Logger::warn(QStringLiteral("[AudioEncoder] 无法打开 Opus 编码器"));
            avcodec_free_context(&m_codecCtx);
            m_codecCtx = nullptr;
            return false;
        }
    }

    // 如果编码器要求的采样格式与 S16 不同，创建重采样上下文。
    if (m_codecCtx->sample_fmt != AV_SAMPLE_FMT_S16) {
        AVChannelLayout chLayout;
        av_channel_layout_default(&chLayout, channels);

        int ret = swr_alloc_set_opts2(&m_swrCtx,
                                       &chLayout, m_codecCtx->sample_fmt, sampleRate,
                                       &chLayout, AV_SAMPLE_FMT_S16,     sampleRate,
                                       0, nullptr);
        if (ret < 0 || swr_init(m_swrCtx) < 0) {
            Logger::warn(QStringLiteral("[AudioEncoder] 重采样初始化失败"));
            close();
            return false;
        }
    }

    m_frameSize = m_codecCtx->frame_size;
    if (m_frameSize <= 0) m_frameSize = 960; // Opus 20ms@48kHz 默认值。

    m_frame = av_frame_alloc();
    m_frame->format      = m_codecCtx->sample_fmt;
    m_frame->sample_rate = sampleRate;
    av_channel_layout_copy(&m_frame->ch_layout, &m_codecCtx->ch_layout);
    m_frame->nb_samples  = m_frameSize;
    av_frame_get_buffer(m_frame, 0);

    m_pkt = av_packet_alloc();
    m_pts = 0;
    m_opened = true;

    Logger::info(QStringLiteral("[AudioEncoder] 已打开: %1 %2Hz %3ch, frame_size=%4")
                 .arg(QString::fromUtf8(codec->name))
                 .arg(sampleRate)
                 .arg(channels)
                 .arg(m_frameSize));
    return true;
}

void AudioEncoder::close()
{
    if (!m_opened) return;
    m_opened = false;

    if (m_pkt)      { av_packet_free(&m_pkt); }
    if (m_frame)    { av_frame_free(&m_frame); }
    if (m_swrCtx)   { swr_free(&m_swrCtx); }
    if (m_codecCtx) { avcodec_free_context(&m_codecCtx); }
}

QByteArray AudioEncoder::encode(const QByteArray &pcmData)
{
    if (!m_opened) return {};

    // 每帧字节数：frame_size * channels * 2（S16LE）。
    int channels = m_codecCtx->ch_layout.nb_channels;
    int bytesPerFrame = m_frameSize * channels * 2; // S16LE = 2 bytes/sample
    if (pcmData.size() < bytesPerFrame) return {};

    av_frame_make_writable(m_frame);
    m_frame->nb_samples = m_frameSize;
    m_frame->pts = m_pts;
    m_pts += m_frameSize;

    if (m_swrCtx) {
        // 需要重采样：S16 → 编码器所需格式。
        const uint8_t *inData[] = { reinterpret_cast<const uint8_t*>(pcmData.constData()) };
        swr_convert(m_swrCtx,
                    m_frame->data, m_frameSize,
                    inData, m_frameSize);
    } else {
        // 直接拷贝 S16 数据到帧。
        memcpy(m_frame->data[0], pcmData.constData(), bytesPerFrame);
    }

    int ret = avcodec_send_frame(m_codecCtx, m_frame);
    if (ret < 0) return {};

    QByteArray result;
    while (avcodec_receive_packet(m_codecCtx, m_pkt) >= 0) {
        result.append(reinterpret_cast<const char*>(m_pkt->data), m_pkt->size);
        av_packet_unref(m_pkt);
    }
    return result;
}
