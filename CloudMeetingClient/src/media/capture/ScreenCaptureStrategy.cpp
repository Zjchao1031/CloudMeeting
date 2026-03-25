/**
 * @file ScreenCaptureStrategy.cpp
 * @brief 实现屏幕共享采集策略（FFmpeg gdigrab）。
 */
#include "media/capture/ScreenCaptureStrategy.h"
#include "common/Constants.h"
#include "common/Logger.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

ScreenCaptureStrategy::ScreenCaptureStrategy() {}

ScreenCaptureStrategy::~ScreenCaptureStrategy()
{
    stop();
}

bool ScreenCaptureStrategy::start()
{
    if (m_running) return true;
    m_running = true;

    QObject::connect(&m_thread, &QThread::started, [this]() { captureLoop(); });
    m_thread.start();
    return true;
}

void ScreenCaptureStrategy::stop()
{
    if (!m_running) return;
    m_running = false;
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait(3000);
    }
    QObject::disconnect(&m_thread, &QThread::started, nullptr, nullptr);
}

bool ScreenCaptureStrategy::isRunning() const
{
    return m_running;
}

void ScreenCaptureStrategy::captureLoop()
{
    avdevice_register_all();

    const AVInputFormat *ifmt = av_find_input_format("gdigrab");
    if (!ifmt) {
        Logger::warn(QStringLiteral("[ScreenCapture] gdigrab 不可用"));
        m_running = false;
        return;
    }

    // 预分配并设置中断回调，使 stop() 能中断阻塞中的 av_read_frame。
    AVFormatContext *fmtCtx = avformat_alloc_context();
    fmtCtx->interrupt_callback.callback = [](void *ctx) -> int {
        return static_cast<std::atomic<bool>*>(ctx)->load() ? 0 : 1;
    };
    fmtCtx->interrupt_callback.opaque = &m_running;

    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "framerate", QByteArray::number(Constants::VIDEO_FPS).constData(), 0);
    av_dict_set(&opts, "draw_mouse", "1", 0);

    // gdigrab 使用 "desktop" 作为输入源名称捕获全屏。
    int ret = avformat_open_input(&fmtCtx, "desktop", ifmt, &opts);
    av_dict_free(&opts);
    if (ret < 0) {
        Logger::warn(QStringLiteral("[ScreenCapture] 无法打开屏幕采集"));
        m_running = false;
        return;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        Logger::warn(QStringLiteral("[ScreenCapture] 无法获取流信息"));
        avformat_close_input(&fmtCtx);
        m_running = false;
        return;
    }

    int videoIdx = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; ++i) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIdx = static_cast<int>(i);
            break;
        }
    }
    if (videoIdx < 0) {
        avformat_close_input(&fmtCtx);
        m_running = false;
        return;
    }

    AVCodecParameters *par = fmtCtx->streams[videoIdx]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(par->codec_id);
    AVCodecContext *decCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(decCtx, par);
    avcodec_open2(decCtx, codec, nullptr);

    // 缩放到目标分辨率并转 RGB32。
    int outW = Constants::VIDEO_WIDTH;
    int outH = Constants::VIDEO_HEIGHT;
    SwsContext *swsCtx = sws_getContext(
        decCtx->width, decCtx->height, decCtx->pix_fmt,
        outW, outH, AV_PIX_FMT_RGB32,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    AVFrame *frame    = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
    int rgbBufSize = av_image_get_buffer_size(AV_PIX_FMT_RGB32, outW, outH, 1);
    uint8_t *rgbBuf = static_cast<uint8_t*>(av_malloc(rgbBufSize));
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize,
                         rgbBuf, AV_PIX_FMT_RGB32, outW, outH, 1);

    AVPacket *pkt = av_packet_alloc();

    Logger::info(QStringLiteral("[ScreenCapture] 开始采集 %1x%2 -> %3x%4")
                 .arg(decCtx->width)
                 .arg(decCtx->height)
                 .arg(outW)
                 .arg(outH));

    while (m_running) {
        ret = av_read_frame(fmtCtx, pkt);
        if (ret < 0) break;

        if (pkt->stream_index == videoIdx) {
            ret = avcodec_send_packet(decCtx, pkt);
            if (ret >= 0) {
                while (avcodec_receive_frame(decCtx, frame) >= 0) {
                    sws_scale(swsCtx,
                              frame->data, frame->linesize, 0, decCtx->height,
                              rgbFrame->data, rgbFrame->linesize);

                    QImage img(rgbFrame->data[0], outW, outH,
                               rgbFrame->linesize[0], QImage::Format_RGB32);
                    if (m_frameCallback)
                        m_frameCallback(img.copy());
                }
            }
        }
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
    av_free(rgbBuf);
    sws_freeContext(swsCtx);
    avcodec_free_context(&decCtx);
    avformat_close_input(&fmtCtx);

    m_running = false;
    Logger::info(QStringLiteral("[ScreenCapture] 采集已停止"));
}
