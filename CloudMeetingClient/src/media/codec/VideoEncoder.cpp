/**
 * @file VideoEncoder.cpp
 * @brief 实现视频编码器封装类（libopenh264 / h264_mf）。
 */
#include "media/codec/VideoEncoder.h"
#include "common/Logger.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

VideoEncoder::VideoEncoder() {}

VideoEncoder::~VideoEncoder() { close(); }

bool VideoEncoder::open(int width, int height, int fps)
{
    if (m_opened) return true;
    m_width  = width;
    m_height = height;

    // 优先使用 libopenh264，其次 h264_mf（MediaFoundation）。
    const AVCodec *codec = avcodec_find_encoder_by_name("libopenh264");
    if (!codec) codec = avcodec_find_encoder_by_name("h264_mf");
    if (!codec) codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        Logger::warn(QStringLiteral("[VideoEncoder] 未找到 H.264 编码器"));
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    m_codecCtx->width     = width;
    m_codecCtx->height    = height;
    m_codecCtx->time_base = {1, fps};
    m_codecCtx->framerate = {fps, 1};
    m_codecCtx->pix_fmt   = AV_PIX_FMT_YUV420P;
    m_codecCtx->gop_size  = fps * 2; // 每 2 秒一个关键帧。
    m_codecCtx->max_b_frames = 0;    // 实时场景不使用 B 帧。
    // 设置 Baseline Profile 以获得最好的兼容性。
    m_codecCtx->profile = 66; // FF_PROFILE_H264_BASELINE
    // 码率控制：局域网场景，给予较高码率。
    m_codecCtx->bit_rate = 2000000; // 2 Mbps

    // 低延迟选项。
    av_opt_set(m_codecCtx->priv_data, "preset", "fast", 0);
    av_opt_set(m_codecCtx->priv_data, "tune", "zerolatency", 0);

    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        Logger::warn(QStringLiteral("[VideoEncoder] 无法打开编码器: %1").arg(QString::fromUtf8(codec->name)));
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
        return false;
    }

    // 创建 RGB32 → YUV420P 转换器。
    m_swsCtx = sws_getContext(
        width, height, AV_PIX_FMT_RGB32,
        width, height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    // 分配 YUV 帧。
    m_yuvFrame = av_frame_alloc();
    m_yuvFrame->format = AV_PIX_FMT_YUV420P;
    m_yuvFrame->width  = width;
    m_yuvFrame->height = height;
    av_frame_get_buffer(m_yuvFrame, 32);

    m_pkt = av_packet_alloc();
    m_pts = 0;
    m_opened = true;

    Logger::info(QStringLiteral("[VideoEncoder] 已打开: %1 %2x%3@%4")
                 .arg(QString::fromUtf8(codec->name))
                 .arg(width)
                 .arg(height)
                 .arg(fps));
    return true;
}

void VideoEncoder::close()
{
    if (!m_opened) return;
    m_opened = false;

    if (m_pkt)      { av_packet_free(&m_pkt); }
    if (m_yuvFrame) { av_frame_free(&m_yuvFrame); }
    if (m_swsCtx)   { sws_freeContext(m_swsCtx); m_swsCtx = nullptr; }
    if (m_codecCtx) { avcodec_free_context(&m_codecCtx); }
}

QByteArray VideoEncoder::encode(const QImage &frame)
{
    if (!m_opened || frame.isNull()) return {};

    // 确保输入为 RGB32 格式。
    QImage img = frame;
    if (img.format() != QImage::Format_RGB32)
        img = img.convertToFormat(QImage::Format_RGB32);

    // RGB32 → YUV420P。
    const uint8_t *srcSlice[] = { img.constBits() };
    int srcStride[] = { static_cast<int>(img.bytesPerLine()) };
    av_frame_make_writable(m_yuvFrame);
    sws_scale(m_swsCtx,
              srcSlice, srcStride, 0, m_height,
              m_yuvFrame->data, m_yuvFrame->linesize);

    m_yuvFrame->pts = m_pts++;

    // 处理强制 IDR 请求。
    if (m_forceIdr) {
        m_yuvFrame->pict_type = AV_PICTURE_TYPE_I;
        m_yuvFrame->flags    |= AV_FRAME_FLAG_KEY;
        m_forceIdr = false;
    } else {
        m_yuvFrame->pict_type = AV_PICTURE_TYPE_NONE;
        m_yuvFrame->flags    &= ~AV_FRAME_FLAG_KEY;
    }

    int ret = avcodec_send_frame(m_codecCtx, m_yuvFrame);
    if (ret < 0) return {};

    QByteArray result;
    while (avcodec_receive_packet(m_codecCtx, m_pkt) >= 0) {
        result.append(reinterpret_cast<const char*>(m_pkt->data), m_pkt->size);
        av_packet_unref(m_pkt);
    }
    return result;
}

void VideoEncoder::forceKeyFrame()
{
    m_forceIdr = true;
}
