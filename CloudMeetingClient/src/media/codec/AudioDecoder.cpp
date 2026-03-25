/**
 * @file AudioDecoder.cpp
 * @brief 实现 Opus 音频解码器封装类。
 */
#include "media/codec/AudioDecoder.h"
#include "common/Logger.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

AudioDecoder::AudioDecoder() {}

AudioDecoder::~AudioDecoder() { close(); }

bool AudioDecoder::open(int sampleRate, int channels)
{
    if (m_opened) return true;
    m_outRate = sampleRate;
    m_outCh   = channels;

    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_OPUS);
    if (!codec) {
        Logger::warn(QStringLiteral("[AudioDecoder] 未找到 Opus 解码器"));
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    m_codecCtx->sample_rate = sampleRate;
    av_channel_layout_default(&m_codecCtx->ch_layout, channels);

    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        Logger::warn(QStringLiteral("[AudioDecoder] 无法打开 Opus 解码器"));
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
        return false;
    }

    m_frame = av_frame_alloc();
    m_pkt   = av_packet_alloc();
    m_opened = true;

    Logger::info(QStringLiteral("[AudioDecoder] 已打开 Opus 解码器 %1Hz %2ch")
                 .arg(sampleRate)
                 .arg(channels));
    return true;
}

void AudioDecoder::close()
{
    if (!m_opened) return;
    m_opened = false;

    if (m_pkt)      { av_packet_free(&m_pkt); }
    if (m_frame)    { av_frame_free(&m_frame); }
    if (m_swrCtx)   { swr_free(&m_swrCtx); }
    if (m_codecCtx) { avcodec_free_context(&m_codecCtx); }
}

QByteArray AudioDecoder::decode(const QByteArray &opusData)
{
    if (!m_opened || opusData.isEmpty()) return {};

    m_pkt->data = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(opusData.constData()));
    m_pkt->size = opusData.size();

    int ret = avcodec_send_packet(m_codecCtx, m_pkt);
    if (ret < 0) return {};

    QByteArray result;
    while (avcodec_receive_frame(m_codecCtx, m_frame) >= 0) {
        int samples = m_frame->nb_samples;

        // 如果解码输出格式已经是 S16，直接拷贝。
        if (m_frame->format == AV_SAMPLE_FMT_S16 && !m_swrCtx) {
            int bytes = samples * m_outCh * 2;
            result.append(reinterpret_cast<const char*>(m_frame->data[0]), bytes);
            continue;
        }

        // 按需创建重采样上下文：解码器输出格式 → S16LE。
        if (!m_swrCtx) {
            AVChannelLayout outLayout;
            av_channel_layout_default(&outLayout, m_outCh);

            ret = swr_alloc_set_opts2(&m_swrCtx,
                                       &outLayout, AV_SAMPLE_FMT_S16, m_outRate,
                                       &m_frame->ch_layout,
                                       static_cast<AVSampleFormat>(m_frame->format),
                                       m_frame->sample_rate,
                                       0, nullptr);
            if (ret < 0 || swr_init(m_swrCtx) < 0) {
                Logger::warn(QStringLiteral("[AudioDecoder] 重采样初始化失败"));
                swr_free(&m_swrCtx);
                return {};
            }
        }

        // 计算输出缓冲区大小并转换。
        int outSamples = swr_get_out_samples(m_swrCtx, samples);
        QByteArray buf(outSamples * m_outCh * 2, Qt::Uninitialized);
        uint8_t *outPtr[] = { reinterpret_cast<uint8_t*>(buf.data()) };

        int converted = swr_convert(m_swrCtx,
                                     outPtr, outSamples,
                                     const_cast<const uint8_t**>(m_frame->data), samples);
        if (converted > 0) {
            buf.resize(converted * m_outCh * 2);
            result.append(buf);
        }
    }

    return result;
}
