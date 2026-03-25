/**
 * @file CameraCaptureStrategy.cpp
 * @brief 实现摄像头采集策略（FFmpeg dshow）。
 */
#include "media/capture/CameraCaptureStrategy.h"
#include "common/Constants.h"
#include "common/Logger.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

CameraCaptureStrategy::CameraCaptureStrategy(const QString &deviceId)
    : m_deviceId(deviceId)
{}

CameraCaptureStrategy::~CameraCaptureStrategy()
{
    stop();
}

void CameraCaptureStrategy::setDeviceId(const QString &deviceId)
{
    QMutexLocker locker(&m_deviceIdMutex);
    m_deviceId = deviceId;
}

bool CameraCaptureStrategy::start()
{
    if (m_running) return true;
    m_running = true;

    // 在工作线程中执行采集循环。
    QThread *t = &m_thread;
    QObject::connect(t, &QThread::started, [this]() { captureLoop(); });
    m_thread.start();
    return true;
}

void CameraCaptureStrategy::stop()
{
    if (!m_running) return;
    m_running = false;
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait(3000);
    }
    // 断开 started 信号，防止下次 start 时重复连接。
    QObject::disconnect(&m_thread, &QThread::started, nullptr, nullptr);
}

bool CameraCaptureStrategy::isRunning() const
{
    return m_running;
}

void CameraCaptureStrategy::captureLoop()
{
    avdevice_register_all();

    const AVInputFormat *ifmt = av_find_input_format("dshow");
    if (!ifmt) {
        Logger::warn(QStringLiteral("[CameraCapture] dshow 不可用"));
        m_running = false;
        return;
    }

    // 在锁内取得设备 ID 的本地局部拷贝，避免往后读取时与主线程竞争。
    QString localDeviceId;
    {
        QMutexLocker locker(&m_deviceIdMutex);
        localDeviceId = m_deviceId;
    }

    // 构造 dshow 设备 URL：video=设备名。
    QString deviceUrl = QStringLiteral("video=%1").arg(localDeviceId);
    QByteArray urlUtf8 = deviceUrl.toUtf8();

    // 预分配并设置中断回调，使 stop() 能中断阻塞中的 av_read_frame。
    AVFormatContext *fmtCtx = avformat_alloc_context();
    fmtCtx->interrupt_callback.callback = [](void *ctx) -> int {
        return static_cast<std::atomic<bool>*>(ctx)->load() ? 0 : 1;
    };
    fmtCtx->interrupt_callback.opaque = &m_running;

    AVDictionary *opts = nullptr;
    // 设置分辨率与帧率。
    av_dict_set(&opts, "video_size",
                QStringLiteral("%1x%2").arg(Constants::VIDEO_WIDTH).arg(Constants::VIDEO_HEIGHT).toUtf8().constData(), 0);
    av_dict_set(&opts, "framerate", QByteArray::number(Constants::VIDEO_FPS).constData(), 0);

    int ret = avformat_open_input(&fmtCtx, urlUtf8.constData(), ifmt, &opts);
    av_dict_free(&opts);
    if (ret < 0) {
        Logger::warn(QStringLiteral("[CameraCapture] 无法打开摄像头: %1").arg(localDeviceId));
        m_running = false;
        return;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        Logger::warn(QStringLiteral("[CameraCapture] 无法获取流信息"));
        avformat_close_input(&fmtCtx);
        m_running = false;
        return;
    }

    // 查找视频流。
    int videoIdx = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; ++i) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIdx = static_cast<int>(i);
            break;
        }
    }
    if (videoIdx < 0) {
        Logger::warn(QStringLiteral("[CameraCapture] 未找到视频流"));
        avformat_close_input(&fmtCtx);
        m_running = false;
        return;
    }

    // 打开解码器（原始采集格式如 rawvideo/mjpeg）。
    AVCodecParameters *par = fmtCtx->streams[videoIdx]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(par->codec_id);
    if (!codec) {
        Logger::warn(QStringLiteral("[CameraCapture] 未找到解码器"));
        avformat_close_input(&fmtCtx);
        m_running = false;
        return;
    }

    AVCodecContext *decCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(decCtx, par);
    if (avcodec_open2(decCtx, codec, nullptr) < 0) {
        Logger::warn(QStringLiteral("[CameraCapture] 无法打开解码器"));
        avcodec_free_context(&decCtx);
        avformat_close_input(&fmtCtx);
        m_running = false;
        return;
    }

    // 创建 swscale 上下文：任意输入格式 → RGB32。
    SwsContext *swsCtx = sws_getContext(
        decCtx->width, decCtx->height, decCtx->pix_fmt,
        decCtx->width, decCtx->height, AV_PIX_FMT_RGB32,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    AVFrame *frame    = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
    int rgbBufSize = av_image_get_buffer_size(AV_PIX_FMT_RGB32, decCtx->width, decCtx->height, 1);
    uint8_t *rgbBuf = static_cast<uint8_t*>(av_malloc(rgbBufSize));
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize,
                         rgbBuf, AV_PIX_FMT_RGB32,
                         decCtx->width, decCtx->height, 1);

    AVPacket *pkt = av_packet_alloc();

    Logger::info(QStringLiteral("[CameraCapture] 开始采集: %1 %2x%3")
                 .arg(localDeviceId)
                 .arg(decCtx->width)
                 .arg(decCtx->height));

    // 采集主循环。
    while (m_running) {
        ret = av_read_frame(fmtCtx, pkt);
        if (ret < 0) break;

        if (pkt->stream_index == videoIdx) {
            ret = avcodec_send_packet(decCtx, pkt);
            if (ret >= 0) {
                while (avcodec_receive_frame(decCtx, frame) >= 0) {
                    // 转换为 RGB32。
                    sws_scale(swsCtx,
                              frame->data, frame->linesize, 0, decCtx->height,
                              rgbFrame->data, rgbFrame->linesize);

                    // 包装为 QImage 并回调。
                    QImage img(rgbFrame->data[0],
                               decCtx->width, decCtx->height,
                               rgbFrame->linesize[0],
                               QImage::Format_RGB32);
                    if (m_frameCallback)
                        m_frameCallback(img.copy()); // 深拷贝，因为缓冲区会被复用。
                }
            }
        }
        av_packet_unref(pkt);
    }

    // 清理资源。
    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
    av_free(rgbBuf);
    sws_freeContext(swsCtx);
    avcodec_free_context(&decCtx);
    avformat_close_input(&fmtCtx);

    m_running = false;
    Logger::info(QStringLiteral("[CameraCapture] 采集已停止"));
}
