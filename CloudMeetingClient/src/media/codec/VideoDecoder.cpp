/**
 * @file VideoDecoder.cpp
 * @brief 实现 H.264 视频解码器封装类。
 */
#include "media/codec/VideoDecoder.h"
#include "common/Logger.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoDecoder::VideoDecoder() {}

VideoDecoder::~VideoDecoder() { close(); }

bool VideoDecoder::open()
{
    if (m_opened) return true;

    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        Logger::warn(QStringLiteral("[VideoDecoder] 未找到 H.264 解码器"));
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        Logger::warn(QStringLiteral("[VideoDecoder] 无法打开解码器"));
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
        return false;
    }

    m_parser = av_parser_init(AV_CODEC_ID_H264);
    m_frame    = av_frame_alloc();
    m_rgbFrame = av_frame_alloc();
    m_pkt      = av_packet_alloc();
    m_opened   = true;

    Logger::info(QStringLiteral("[VideoDecoder] 已打开 H.264 解码器"));
    return true;
}

void VideoDecoder::close()
{
    if (!m_opened) return;
    m_opened = false;

    if (m_pkt)      { av_packet_free(&m_pkt); }
    if (m_frame)    { av_frame_free(&m_frame); }
    if (m_rgbFrame) { av_frame_free(&m_rgbFrame); }
    if (m_rgbBuf)   { av_free(m_rgbBuf); m_rgbBuf = nullptr; }
    if (m_swsCtx)   { sws_freeContext(m_swsCtx); m_swsCtx = nullptr; }
    if (m_parser)   { av_parser_close(m_parser); m_parser = nullptr; }
    if (m_codecCtx) { avcodec_free_context(&m_codecCtx); }
    m_swsW = m_swsH = 0;
}

QImage VideoDecoder::decode(const QByteArray &h264Data)
{
    if (!m_opened || h264Data.isEmpty()) return {};

    QImage lastImg;
    const uint8_t *data = reinterpret_cast<const uint8_t*>(h264Data.constData());
    int dataSize = h264Data.size();

    // 使用 parser 从码流中提取完整 NALU 并逐个送入解码器。
    while (dataSize > 0) {
        int parsedLen = av_parser_parse2(
            m_parser, m_codecCtx,
            &m_pkt->data, &m_pkt->size,
            data, dataSize,
            AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

        data     += parsedLen;
        dataSize -= parsedLen;

        if (m_pkt->size <= 0) continue;

        int ret = avcodec_send_packet(m_codecCtx, m_pkt);
        if (ret < 0) continue;

        while (avcodec_receive_frame(m_codecCtx, m_frame) >= 0) {
            int w = m_frame->width;
            int h = m_frame->height;

            // 按需重建 swscale 上下文（分辨率可能变化）。
            if (w != m_swsW || h != m_swsH) {
                if (m_swsCtx) sws_freeContext(m_swsCtx);
                if (m_rgbBuf) av_free(m_rgbBuf);

                m_swsCtx = sws_getContext(
                    w, h, static_cast<AVPixelFormat>(m_frame->format),
                    w, h, AV_PIX_FMT_RGB32,
                    SWS_BILINEAR, nullptr, nullptr, nullptr);

                int bufSize = av_image_get_buffer_size(AV_PIX_FMT_RGB32, w, h, 1);
                m_rgbBuf = static_cast<uint8_t*>(av_malloc(bufSize));
                av_image_fill_arrays(m_rgbFrame->data, m_rgbFrame->linesize,
                                     m_rgbBuf, AV_PIX_FMT_RGB32, w, h, 1);
                m_swsW = w;
                m_swsH = h;
            }

            sws_scale(m_swsCtx,
                      m_frame->data, m_frame->linesize, 0, h,
                      m_rgbFrame->data, m_rgbFrame->linesize);

            // 深拷贝为 QImage。
            lastImg = QImage(m_rgbFrame->data[0], w, h,
                             m_rgbFrame->linesize[0],
                             QImage::Format_RGB32).copy();
        }
    }

    return lastImg;
}
