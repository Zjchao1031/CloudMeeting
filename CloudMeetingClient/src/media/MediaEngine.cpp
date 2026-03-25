/**
 * @file MediaEngine.cpp
 * @brief 实现媒体能力调度引擎。
 */
#include "media/MediaEngine.h"
#include "media/capture/CameraCaptureStrategy.h"
#include "media/capture/ScreenCaptureStrategy.h"
#include "media/codec/VideoEncoder.h"
#include "media/codec/VideoDecoder.h"
#include "media/codec/AudioEncoder.h"
#include "media/codec/AudioDecoder.h"
#include "network/NetworkFacade.h"
#include "network/MediaUdpClient.h"
#include "common/Constants.h"
#include "common/Logger.h"
#include <QAudioFormat>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
}

MediaEngine::MediaEngine(QObject *parent)
    : QObject(parent)
    , m_cameraCapture(std::make_unique<CameraCaptureStrategy>())
    , m_screenCapture(std::make_unique<ScreenCaptureStrategy>())
    , m_cameraEncoder(std::make_unique<VideoEncoder>())
    , m_screenEncoder(std::make_unique<VideoEncoder>())
    , m_audioEncoder(std::make_unique<AudioEncoder>())
    , m_audioDecoder(std::make_unique<AudioDecoder>())
{}

MediaEngine::~MediaEngine()
{
    stopAll();
}

void MediaEngine::setNetworkFacade(NetworkFacade *network, const QString &localUserId)
{
    m_network     = network;
    m_localUserId = localUserId;

    if (!m_network) return;

    auto *media = m_network->mediaClient();
    if (media) {
        connect(media, &MediaUdpClient::videoDataReceived,
                this, [this](quint32 userId, QByteArray data, bool /*isCamera*/) {
            onRemoteVideoData(QString::number(userId), data);
        });
        connect(media, &MediaUdpClient::audioDataReceived,
                this, [this](quint32 userId, QByteArray data) {
            onRemoteAudioData(QString::number(userId), data);
        });
    }
}

// ─── 视频采集 ────────────────────────────────────────────────

void MediaEngine::startCameraCapture(const QString &cameraId)
{
    // 打开摄像头专用编码器。
    m_cameraEncoder->open(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT, Constants::VIDEO_FPS);

    auto *cam = dynamic_cast<CameraCaptureStrategy*>(m_cameraCapture.get());
    if (cam && !cameraId.isEmpty())
        cam->setDeviceId(cameraId);

    m_cameraCapture->setFrameCallback([this](const QImage &frame) {
        onCapturedVideoFrame(frame, true);
    });
    m_cameraCapture->start();
    Logger::info(QStringLiteral("[MediaEngine] 摄像头采集已启动"));
}

void MediaEngine::stopCameraCapture()
{
    m_cameraCapture->stop();
    Logger::info(QStringLiteral("[MediaEngine] 摄像头采集已停止"));
}

void MediaEngine::startScreenShare()
{
    // 打开屏幕共享专用编码器。
    m_screenEncoder->open(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT, Constants::VIDEO_FPS);

    m_screenCapture->setFrameCallback([this](const QImage &frame) {
        onCapturedVideoFrame(frame, false);
    });
    m_screenCapture->start();
    Logger::info(QStringLiteral("[MediaEngine] 屏幕共享已启动"));
}

void MediaEngine::stopScreenShare()
{
    m_screenCapture->stop();
    Logger::info(QStringLiteral("[MediaEngine] 屏幕共享已停止"));
}

void MediaEngine::onCapturedVideoFrame(const QImage &frame, bool isCamera)
{
    // 发送本地预览信号。
    emit localVideoFrame(frame);

    // 根据来源选择对应的编码器。
    VideoEncoder &enc = isCamera ? *m_cameraEncoder : *m_screenEncoder;
    QByteArray h264 = enc.encode(frame);
    if (h264.isEmpty() || !m_network) return;

    // 通过 UDP 发送——必须在主线程调用 QUdpSocket，使用队列连接切换线程。
    quint32 uid    = m_localUserId.toUInt();
    bool    screen = !isCamera;
    QMetaObject::invokeMethod(this, [this, uid, h264, screen]() {
        if (m_network)
            m_network->mediaClient()->sendVideoFrameFragments(uid, h264, screen);
    }, Qt::QueuedConnection);
}

// ─── 音频采集 ────────────────────────────────────────────────

void MediaEngine::startAudioCapture(const QString &micId)
{
    if (m_audioRunning) return;

    m_audioEncoder->open(Constants::AUDIO_SAMPLE_RATE, Constants::AUDIO_CHANNELS);
    m_micId = micId;
    m_audioRunning = true;

    QObject::connect(&m_audioCaptureThread, &QThread::started,
                     [this]() { audioCaptureLoop(); });
    m_audioCaptureThread.start();
    Logger::info(QStringLiteral("[MediaEngine] 麦克风采集已启动"));
}

void MediaEngine::stopAudioCapture()
{
    if (!m_audioRunning) return;
    m_audioRunning = false;

    if (m_audioCaptureThread.isRunning()) {
        m_audioCaptureThread.quit();
        m_audioCaptureThread.wait(3000);
    }
    QObject::disconnect(&m_audioCaptureThread, &QThread::started, nullptr, nullptr);
    Logger::info(QStringLiteral("[MediaEngine] 麦克风采集已停止"));
}

void MediaEngine::audioCaptureLoop()
{
    avdevice_register_all();

    const AVInputFormat *ifmt = av_find_input_format("dshow");
    if (!ifmt) {
        Logger::warn(QStringLiteral("[MediaEngine] dshow 不可用，无法采集音频"));
        m_audioRunning = false;
        return;
    }

    // 构造 dshow 音频设备 URL。
    QString url = QStringLiteral("audio=%1").arg(m_micId);
    QByteArray urlUtf8 = url.toUtf8();

    // 预分配并设置中断回调，使 stopAudioCapture() 能中断阻塞中的 av_read_frame。
    AVFormatContext *fmtCtx = avformat_alloc_context();
    fmtCtx->interrupt_callback.callback = [](void *ctx) -> int {
        return static_cast<std::atomic<bool>*>(ctx)->load() ? 0 : 1;
    };
    fmtCtx->interrupt_callback.opaque = &m_audioRunning;

    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "sample_rate", QByteArray::number(Constants::AUDIO_SAMPLE_RATE).constData(), 0);
    av_dict_set(&opts, "channels", QByteArray::number(Constants::AUDIO_CHANNELS).constData(), 0);

    int ret = avformat_open_input(&fmtCtx, urlUtf8.constData(), ifmt, &opts);
    av_dict_free(&opts);
    if (ret < 0) {
        Logger::warn(QStringLiteral("[MediaEngine] 无法打开麦克风: %1").arg(m_micId));
        m_audioRunning = false;
        return;
    }

    avformat_find_stream_info(fmtCtx, nullptr);

    int audioIdx = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; ++i) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioIdx = static_cast<int>(i);
            break;
        }
    }
    if (audioIdx < 0) {
        avformat_close_input(&fmtCtx);
        m_audioRunning = false;
        return;
    }

    AVCodecParameters *par = fmtCtx->streams[audioIdx]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(par->codec_id);
    AVCodecContext *decCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(decCtx, par);
    avcodec_open2(decCtx, codec, nullptr);

    // 重采样：采集格式 → S16LE mono 48kHz。
    SwrContext *swr = nullptr;
    AVChannelLayout outLayout;
    av_channel_layout_default(&outLayout, Constants::AUDIO_CHANNELS);
    swr_alloc_set_opts2(&swr,
                         &outLayout, AV_SAMPLE_FMT_S16, Constants::AUDIO_SAMPLE_RATE,
                         &decCtx->ch_layout, decCtx->sample_fmt, decCtx->sample_rate,
                         0, nullptr);
    swr_init(swr);

    AVFrame *frame = av_frame_alloc();
    AVPacket *pkt  = av_packet_alloc();

    // 每帧 960 采样（20ms@48kHz），字节数 = 960 * 1 * 2。
    const int frameSamples = 960;
    const int frameBytes   = frameSamples * Constants::AUDIO_CHANNELS * 2;
    QByteArray pcmBuffer;

    Logger::info(QStringLiteral("[MediaEngine] 音频采集循环开始"));

    while (m_audioRunning) {
        ret = av_read_frame(fmtCtx, pkt);
        if (ret < 0) break;

        if (pkt->stream_index == audioIdx) {
            avcodec_send_packet(decCtx, pkt);
            while (avcodec_receive_frame(decCtx, frame) >= 0) {
                // 重采样到 S16LE。
                int outSamples = swr_get_out_samples(swr, frame->nb_samples);
                QByteArray tmp(outSamples * Constants::AUDIO_CHANNELS * 2, Qt::Uninitialized);
                uint8_t *outPtr[] = { reinterpret_cast<uint8_t*>(tmp.data()) };
                int converted = swr_convert(swr, outPtr, outSamples,
                                             const_cast<const uint8_t**>(frame->data),
                                             frame->nb_samples);
                if (converted > 0) {
                    tmp.resize(converted * Constants::AUDIO_CHANNELS * 2);

                    // 应用音量增益。
                    if (m_captureVolume != 100) {
                        auto *samples = reinterpret_cast<int16_t*>(tmp.data());
                        int count = tmp.size() / 2;
                        double gain = m_captureVolume / 100.0;
                        for (int i = 0; i < count; ++i) {
                            int val = static_cast<int>(samples[i] * gain);
                            samples[i] = static_cast<int16_t>(qBound(-32768, val, 32767));
                        }
                    }

                    pcmBuffer.append(tmp);
                }
            }
        }
        av_packet_unref(pkt);

        // 凑够一帧就编码并发送。
        while (pcmBuffer.size() >= frameBytes) {
            QByteArray pcmFrame = pcmBuffer.left(frameBytes);
            pcmBuffer.remove(0, frameBytes);

            QByteArray opus = m_audioEncoder->encode(pcmFrame);
            if (!opus.isEmpty()) {
                quint32 uid = m_localUserId.toUInt();
                // 通过队列连接切换回主线程，避免跨线程操作 QUdpSocket。
                QMetaObject::invokeMethod(this, [this, uid, opus]() {
                    if (m_network)
                        m_network->mediaClient()->sendAudioFrame(uid, opus);
                }, Qt::QueuedConnection);
            }
        }
    }

    av_packet_free(&pkt);
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_free_context(&decCtx);
    avformat_close_input(&fmtCtx);

    m_audioRunning = false;
    Logger::info(QStringLiteral("[MediaEngine] 音频采集循环结束"));
}

// ─── 远端媒体接收 ─────────────────────────────────────────────

VideoDecoder* MediaEngine::ensureVideoDecoder(const QString &userId)
{
    if (!m_videoDecoders.count(userId)) {
        auto dec = std::make_unique<VideoDecoder>();
        dec->open();
        m_videoDecoders[userId] = std::move(dec);
    }
    return m_videoDecoders[userId].get();
}

void MediaEngine::onRemoteVideoData(const QString &userId, const QByteArray &h264Data)
{
    VideoDecoder *dec = ensureVideoDecoder(userId);
    QImage img = dec->decode(h264Data);
    if (!img.isNull())
        emit remoteVideoFrame(userId, img);
}

void MediaEngine::onRemoteAudioData(const QString &userId, const QByteArray &opusData)
{
    Q_UNUSED(userId)

    m_audioDecoder->open(Constants::AUDIO_SAMPLE_RATE, Constants::AUDIO_CHANNELS);

    QByteArray pcm = m_audioDecoder->decode(opusData);
    if (pcm.isEmpty()) return;

    // 应用播放音量。
    if (m_playbackVolume != 100) {
        auto *samples = reinterpret_cast<int16_t*>(pcm.data());
        int count = pcm.size() / 2;
        double gain = m_playbackVolume / 100.0;
        for (int i = 0; i < count; ++i) {
            int val = static_cast<int>(samples[i] * gain);
            samples[i] = static_cast<int16_t>(qBound(-32768, val, 32767));
        }
    }

    // 按需创建 QAudioSink。
    if (!m_audioSink) {
        QAudioFormat fmt;
        fmt.setSampleRate(Constants::AUDIO_SAMPLE_RATE);
        fmt.setChannelCount(Constants::AUDIO_CHANNELS);
        fmt.setSampleFormat(QAudioFormat::Int16);

        QAudioDevice dev = QMediaDevices::defaultAudioOutput();
        if (dev.isFormatSupported(fmt)) {
            m_audioSink = std::make_unique<QAudioSink>(dev, fmt);
            m_audioIO = m_audioSink->start();
        }
    }

    if (m_audioIO)
        m_audioIO->write(pcm);
}

// ─── 音量控制 ─────────────────────────────────────────────────

void MediaEngine::setCaptureVolume(int value)
{
    m_captureVolume = qBound(0, value, 100);
}

void MediaEngine::setPlaybackVolume(int value)
{
    m_playbackVolume = qBound(0, value, 100);
}

void MediaEngine::forceVideoKeyFrame()
{
    m_cameraEncoder->forceKeyFrame();
    m_screenEncoder->forceKeyFrame();
}

void MediaEngine::setLocalUserId(const QString &userId)
{
    m_localUserId = userId;
}

void MediaEngine::removeUserDecoder(const QString &userId)
{
    if (m_videoDecoders.count(userId)) {
        m_videoDecoders.erase(userId);
        Logger::info(QStringLiteral("[MediaEngine] 已移除用户 %1 的视频解码器").arg(userId));
    }
}

void MediaEngine::stopAll()
{
    stopAudioCapture();
    stopCameraCapture();
    stopScreenShare();

    m_cameraEncoder->close();
    m_screenEncoder->close();
    m_audioEncoder->close();

    // 释放全部远端用户解码器。
    m_videoDecoders.clear();

    m_audioDecoder->close();

    if (m_audioSink) {
        m_audioSink->stop();
        m_audioSink.reset();
        m_audioIO = nullptr;
    }
}
