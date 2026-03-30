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
#include <QAudioDevice>
#include <QDateTime>

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
                this, [this](quint32 userId, QByteArray data, bool isCamera) {
            onRemoteVideoData(QString::number(userId), data, isCamera);
        });
        connect(media, &MediaUdpClient::audioDataReceived,
                this, [this](quint32 userId, QByteArray data) {
            onRemoteAudioData(QString::number(userId), data);
        });
        // 分片重组超时：向对应发送方请求关键帧。
        connect(media, &MediaUdpClient::fragmentTimeout,
                this, [this](quint32 userId) {
            tryRequestKeyframe(QString::number(userId));
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
    // 在后端线程内做帧率节流，避免超频帧以 QueuedConnection 泛洪主线程事件队列。
    constexpr qint64 kFrameIntervalMs = 1000 / Constants::VIDEO_FPS;
    auto &lastMs = isCamera ? m_lastCameraFrameMs : m_lastScreenFrameMs;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - lastMs.load(std::memory_order_relaxed) < kFrameIntervalMs) return;
    lastMs.store(now, std::memory_order_relaxed);

    // 发送本地预览信号（携带来源类型，供显示层区分渲染）。
    emit localVideoFrame(frame, isCamera);

    // 根据来源选择对应的编码器。
    VideoEncoder &enc = isCamera ? *m_cameraEncoder : *m_screenEncoder;
    QByteArray h264 = enc.encode(frame);
    if (h264.isEmpty() || !m_network) return;

    // 通过 UDP 发送——必须在主线程调用 QUdpSocket，使用队列连接切换线程。
    quint32 uid   = m_localNumericId;
    bool    isCam = isCamera;
    QMetaObject::invokeMethod(this, [this, uid, h264, isCam]() {
        if (m_network)
            m_network->mediaClient()->sendVideoFrameFragments(uid, h264, isCam);
    }, Qt::QueuedConnection);
}

// ─── 音频采集 ────────────────────────────────────────────────

void MediaEngine::startAudioCapture(const QString &micId)
{
    if (m_audioSource) return;

    m_audioEncoder->open(Constants::AUDIO_SAMPLE_RATE, Constants::AUDIO_CHANNELS);
    m_micId = micId;

    // 按名称查找设备，找不到则使用系统默认输入。
    QAudioDevice device = QMediaDevices::defaultAudioInput();
    if (!micId.isEmpty()) {
        for (const QAudioDevice &dev : QMediaDevices::audioInputs()) {
            if (dev.description() == micId) {
                device = dev;
                break;
            }
        }
    }

    // 格式协商：优先 S16 mono 48kHz；若不支持则依次降级。
    // onAudioInputReady 会将任意格式软件转换为 S16LE mono。
    auto tryFmt = [&](int ch, QAudioFormat::SampleFormat sf) -> QAudioFormat {
        QAudioFormat f;
        f.setSampleRate(Constants::AUDIO_SAMPLE_RATE);
        f.setChannelCount(ch);
        f.setSampleFormat(sf);
        return f;
    };

    QAudioFormat fmt;
    if      (device.isFormatSupported(tryFmt(1, QAudioFormat::Int16)))   fmt = tryFmt(1, QAudioFormat::Int16);
    else if (device.isFormatSupported(tryFmt(2, QAudioFormat::Int16)))   fmt = tryFmt(2, QAudioFormat::Int16);
    else if (device.isFormatSupported(tryFmt(1, QAudioFormat::Float)))   fmt = tryFmt(1, QAudioFormat::Float);
    else if (device.isFormatSupported(tryFmt(2, QAudioFormat::Float)))   fmt = tryFmt(2, QAudioFormat::Float);
    else                                                                  fmt = device.preferredFormat();

    m_audioCaptureFormat = fmt;
    Logger::info(QStringLiteral("[MediaEngine] 麦克风格式: %1Hz %2ch %3")
                 .arg(fmt.sampleRate()).arg(fmt.channelCount())
                 .arg(fmt.sampleFormat() == QAudioFormat::Int16 ? "Int16" : "Float32"));

    m_audioSource = new QAudioSource(device, fmt, this);
    m_audioCaptureIO = m_audioSource->start();

    if (m_audioSource->error() != QAudio::NoError || !m_audioCaptureIO) {
        Logger::warn(QStringLiteral("[MediaEngine] 无法打开麦克风: %1").arg(device.description()));
        delete m_audioSource;
        m_audioSource    = nullptr;
        m_audioCaptureIO = nullptr;
        return;
    }

    connect(m_audioCaptureIO, &QIODevice::readyRead, this, &MediaEngine::onAudioInputReady);
    Logger::info(QStringLiteral("[MediaEngine] 麦克风采集已启动: %1").arg(device.description()));
}

void MediaEngine::stopAudioCapture()
{
    if (!m_audioSource) return;

    if (m_audioCaptureIO) {
        disconnect(m_audioCaptureIO, &QIODevice::readyRead, this, &MediaEngine::onAudioInputReady);
        m_audioCaptureIO = nullptr;
    }
    m_audioSource->stop();
    delete m_audioSource;
    m_audioSource = nullptr;
    m_audioPcmBuffer.clear();
    Logger::info(QStringLiteral("[MediaEngine] 麦克风采集已停止"));
}

void MediaEngine::onAudioInputReady()
{
    if (!m_audioCaptureIO) return;

    const QByteArray raw = m_audioCaptureIO->readAll();
    if (raw.isEmpty()) return;

    // 转换为 S16LE mono（编码器输入格式）并应用增益。
    m_audioPcmBuffer.append(convertToS16Mono(raw, m_audioCaptureFormat));

    // 每帧 960 采样（20ms@48kHz）凑满后编码发送。
    const int frameBytes = 960 * Constants::AUDIO_CHANNELS * 2;
    while (m_audioPcmBuffer.size() >= frameBytes) {
        QByteArray pcmFrame = m_audioPcmBuffer.left(frameBytes);
        m_audioPcmBuffer.remove(0, frameBytes);

        QByteArray opus = m_audioEncoder->encode(pcmFrame);
        if (!opus.isEmpty() && m_network) {
            m_network->mediaClient()->sendAudioFrame(m_localNumericId, opus);
        }
    }
}

QByteArray MediaEngine::convertToS16Mono(const QByteArray &raw, const QAudioFormat &srcFmt)
{
    const int inCh  = srcFmt.channelCount();
    const bool isFloat = (srcFmt.sampleFormat() == QAudioFormat::Float);
    const double gain  = m_captureVolume / 100.0;

    // 每个输入帧的字节数（一个声道一个采样）。
    const int bytesPerSample = isFloat ? 4 : 2;
    const int frameSize      = bytesPerSample * inCh;
    const int frameCount     = raw.size() / frameSize;

    QByteArray out(frameCount * 2, Qt::Uninitialized);
    auto *dst = reinterpret_cast<int16_t*>(out.data());

    for (int i = 0; i < frameCount; ++i) {
        float mono = 0.0f;
        if (isFloat) {
            const auto *src = reinterpret_cast<const float*>(raw.constData() + i * frameSize);
            for (int ch = 0; ch < inCh; ++ch) mono += src[ch];
            mono /= inCh;
        } else {
            const auto *src = reinterpret_cast<const int16_t*>(raw.constData() + i * frameSize);
            int32_t sum = 0;
            for (int ch = 0; ch < inCh; ++ch) sum += src[ch];
            mono = static_cast<float>(sum) / inCh;
        }
        // Float 采样在 [-1.0,1.0] 范围，需先缩放至整数范围再应用增益。
        // Int16 采样已在 [-32768,32767] 范围，直接应用增益即可。
        const float scaled = isFloat ? (mono * 32767.0f) : mono;
        const int val = static_cast<int>(scaled * gain);
        dst[i] = static_cast<int16_t>(qBound(-32768, val, 32767));
    }
    return out;
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

void MediaEngine::onRemoteVideoData(const QString &userId, const QByteArray &h264Data, bool isCamera)
{
    // 使用复合 key 区分同一用户的摄像头流与屏幕共享流解码器。
    const QString decoderKey = userId + (isCamera ? QStringLiteral("_cam") : QStringLiteral("_scr"));
    VideoDecoder *dec = ensureVideoDecoder(decoderKey);
    QImage img = dec->decode(h264Data);
    if (!img.isNull()) {
        m_decodeFailCount[decoderKey] = 0; // 解码成功，重置该路流的连续失败计数。
        emit remoteVideoFrame(userId, img, isCamera);
    } else {
        // 连续解码失败达到阈值时向发送方请求关键帧。
        if (++m_decodeFailCount[decoderKey] >= Constants::VIDEO_KEYFRAME_FAIL_THRESHOLD) {
            m_decodeFailCount[decoderKey] = 0;
            tryRequestKeyframe(userId);
        }
    }
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

        // 若设备不支持单声道，降级尝试双声道（写入时再将 PCM 上混）。
        if (!dev.isFormatSupported(fmt)) {
            fmt.setChannelCount(2);
            Logger::warn(QStringLiteral("[MediaEngine] 输出设备不支持单声道，降级为双声道"));
        }

        m_audioSink = std::make_unique<QAudioSink>(dev, fmt);
        m_audioIO   = m_audioSink->start();

        if (m_audioSink->error() != QAudio::NoError || !m_audioIO) {
            Logger::warn(QStringLiteral("[MediaEngine] 无法打开音频播放设备: %1").arg(dev.description()));
            m_audioSink.reset();
            m_audioIO = nullptr;
        } else {
            Logger::info(QStringLiteral("[MediaEngine] 音频播放已开始: %1Hz %2ch")
                         .arg(fmt.sampleRate()).arg(fmt.channelCount()));
        }
    }

    if (m_audioIO) {
        // 若播放格式为双声道但解码 PCM 是单声道，执行上混。
        const int sinkChannels = m_audioSink->format().channelCount();
        if (sinkChannels == 2 && Constants::AUDIO_CHANNELS == 1) {
            QByteArray stereo(pcm.size() * 2, Qt::Uninitialized);
            auto       *dst = reinterpret_cast<int16_t *>(stereo.data());
            const auto *src = reinterpret_cast<const int16_t *>(pcm.constData());
            const int   cnt = pcm.size() / 2;
            for (int i = 0; i < cnt; ++i) {
                dst[i * 2]     = src[i];
                dst[i * 2 + 1] = src[i];
            }
            m_audioIO->write(stereo);
        } else {
            m_audioIO->write(pcm);
        }
    }
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

void MediaEngine::setLocalNumericId(quint32 numericId)
{
    m_localNumericId = numericId;
}

void MediaEngine::removeUserDecoder(const QString &userId)
{
    // 同时移除摄像头流和屏幕共享流两个解码器。
    const QStringList keys = { userId + QStringLiteral("_cam"), userId + QStringLiteral("_scr") };
    for (const QString &key : keys) {
        if (m_videoDecoders.count(key))
            m_videoDecoders.erase(key);
    }
    Logger::info(QStringLiteral("[MediaEngine] 已移除用户 %1 的视频解码器").arg(userId));
}

void MediaEngine::tryRequestKeyframe(const QString &userId)
{
    constexpr qint64 COOLDOWN_MS = 2000; // 2 秒冷却，防止泛洪。
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastKeyframeReqMs.value(userId, 0) < COOLDOWN_MS) return;
    m_lastKeyframeReqMs[userId] = now;
    if (m_network)
        m_network->sendRequestKeyframe(userId);
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
