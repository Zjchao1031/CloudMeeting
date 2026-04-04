#pragma once
#include <QObject>
#include <QImage>
#include <QByteArray>
#include <QAudioSink>
#include <QAudioSource>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QString>
#include <QHash>
#include <memory>
#include <unordered_map>
#include <atomic>

/**
 * @file MediaEngine.h
 * @brief 声明媒体能力调度引擎。
 */

/**
 * @struct QStringHasher
 * @brief 为 std::unordered_map 提供 QString 键的哈希函数。
 */
struct QStringHasher {
    std::size_t operator()(const QString &s) const noexcept {
        return static_cast<std::size_t>(qHash(s));
    }
};

class IVideoCaptureStrategy;
class VideoEncoder;
class VideoDecoder;
class AudioEncoder;
class AudioDecoder;
class NetworkFacade;

/**
 * @class MediaEngine
 * @brief 负责统一管理采集、编码、解码、渲染的启停与线程生命周期。
 *
 * 暴露简洁接口供 MeetingController 调用，内部协调采集→编码→网络发送
 * 以及网络接收→解码→渲染/播放的完整媒体流水线。
 */
class MediaEngine : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造媒体引擎。
     * @param[in] parent 父对象。
     */
    explicit MediaEngine(QObject *parent = nullptr);

    /**
     * @brief 析构媒体引擎并停止正在运行的媒体采集。
     */
    ~MediaEngine() override;

    /**
     * @brief 绑定网络门面，用于发送编码后的媒体数据和接收远端媒体。
     * @param[in] network 网络门面指针。
     * @param[in] localUserId 本地用户 ID。
     */
    void setNetworkFacade(NetworkFacade *network, const QString &localUserId);

    /**
     * @brief 启动摄像头采集与视频编码。
     * @param[in] cameraId 摄像头设备标识。
     */
    void startCameraCapture(const QString &cameraId = {});

    /**
     * @brief 停止摄像头采集。
     */
    void stopCameraCapture();

    /**
     * @brief 启动屏幕共享采集与视频编码。
     */
    void startScreenShare();

    /**
     * @brief 停止屏幕共享采集。
     */
    void stopScreenShare();

    /**
     * @brief 启动麦克风采集与音频编码。
     * @param[in] micId 麦克风设备标识。
     */
    void startAudioCapture(const QString &micId = {});

    /**
     * @brief 停止麦克风采集。
     */
    void stopAudioCapture();

    /**
     * @brief 设置采集音量增益。
     * @param[in] value 采集音量值，范围为 0 到 100。
     */
    void setCaptureVolume(int value);

    /**
     * @brief 设置播放音量。
     * @param[in] value 播放音量值，范围为 0 到 100。
     */
    void setPlaybackVolume(int value);

    /**
     * @brief 请求视频编码器强制输出 IDR 关键帧。
     * @param[in] isCamera 是否为摄像头流（否则为屏幕共享流）。
     */
    void forceVideoKeyFrame(bool isCamera);

    /**
     * @brief 设置本地用户 ID，用于标识发送的媒体数据包。
     * @param[in] userId 本地用户 UUID 字符串。
     */
    void setLocalUserId(const QString &userId);

    /**
     * @brief 设置本地用户的数字 ID，填入 UDP 媒体包头以供服务器转发识别。
     * @param[in] numericId 服务器分配的数字 ID。
     */
    void setLocalNumericId(quint32 numericId);

    /**
     * @brief 移除指定远端用户的全部视频解码器（摄像头流与屏幕共享流），在成员离开时调用以释放资源。
     * @param[in] userId 将要清除解码器的用户 ID。
     */
    void removeUserDecoder(const QString &userId);

    /**
     * @brief 停止所有采集、编码并释放资源。
     */
    void stopAll();

signals:
    /**
     * @brief 解码出远端视频帧时发出。
     * @param[in] userId 视频帧所属用户。
     * @param[in] frame 解码后的视频图像。
     * @param[in] isCamera 是否为摄像头流（否则为屏幕共享流）。
     */
    void remoteVideoFrame(const QString &userId, const QImage &frame, bool isCamera);

    /**
     * @brief 本地预览视频帧时发出。
     * @param[in] frame 本地采集的视频图像。
     * @param[in] isCamera 是否为摄像头流（否则为屏幕共享流）。
     */
    void localVideoFrame(const QImage &frame, bool isCamera);

private slots:
    /**
     * @brief 处理从网络接收到的远端视频数据。
     * @param[in] userId 发送方用户 ID。
     * @param[in] h264Data H.264 编码数据。
     * @param[in] isCamera 是否为摄像头流（否则为屏幕共享流）。
     */
    void onRemoteVideoData(const QString &userId, const QByteArray &h264Data, bool isCamera);

    /**
     * @brief 处理从网络接收到的远端音频数据。
     * @param[in] userId 发送方用户 ID。
     * @param[in] opusData Opus 编码数据。
     */
    void onRemoteAudioData(const QString &userId, const QByteArray &opusData);

private:
    /**
     * @brief 按需获取或创建指定用户的视频解码器。
     * @param[in] userId 目标用户 ID。
     * @return 对应的视频解码器指针。
     */
    VideoDecoder* ensureVideoDecoder(const QString &userId);

private:
    /**
     * @brief 视频帧采集回调：编码并通过网络发送。
     * @param[in] frame 采集到的 RGB32 视频帧。
     * @param[in] isCamera 是否来自摄像头（否则为屏幕共享）。
     */
    void onCapturedVideoFrame(const QImage &frame, bool isCamera);

    /**
     * @brief QAudioSource 有新音频数据可读时被调用：应用音量增益、凸凹 Opus 帧并发送。
     */
    void onAudioInputReady();

    /**
     * @brief 将采集到的原生 PCM 转换为 S16LE mono，同时应用音量增益。
     * @param[in] raw     原始采样数据。
     * @param[in] srcFmt  原始格式描述符。
     * @return 转换后的 S16LE mono 数据。
     */
    QByteArray convertToS16Mono(const QByteArray &raw, const QAudioFormat &srcFmt);

    /**
     * @brief 带 2 秒冷却的关键帧请求辅助：若距上次请求未超过冷却时间则忽略。
     * @param[in] userId 需要请求关键帧的远端用户 ID（UUID 字符串）。
     * @param[in] isCamera 是否为摄像头流（否则为屏幕共享流）。
     */
    void tryRequestKeyframe(const QString &userId, bool isCamera);

    NetworkFacade *m_network       = nullptr; ///< 绑定的网络门面。
    QString        m_localUserId;              ///< 本地用户 UUID。
    quint32        m_localNumericId = 0;       ///< 本地用户数字 ID（UDP 包头用）。

    // 视频采集。
    std::unique_ptr<IVideoCaptureStrategy> m_cameraCapture;  ///< 摄像头采集策略。
    std::unique_ptr<IVideoCaptureStrategy> m_screenCapture;  ///< 屏幕共享采集策略。

    // 编码器（摄像头与屏幕共享各持独立实例）。
    std::unique_ptr<VideoEncoder> m_cameraEncoder; ///< 摄像头流 H.264 视频编码器。
    std::unique_ptr<VideoEncoder> m_screenEncoder; ///< 屏幕共享流 H.264 视频编码器。
    std::unique_ptr<AudioEncoder> m_audioEncoder;  ///< Opus 音频编码器。

    // 解码器（每个远端用户一个，延迟创建）。
    std::unordered_map<QString, std::unique_ptr<VideoDecoder>, QStringHasher> m_videoDecoders; ///< 远端用户 ID → H.264 解码器映射表。
    std::unique_ptr<AudioDecoder> m_audioDecoder;  ///< Opus 音频解码器（混音播放）。

    // 音频采集（Qt QAudioSource）。
    QAudioSource *m_audioSource      = nullptr; ///< Qt 麦克风采集对象。
    QIODevice    *m_audioCaptureIO   = nullptr; ///< QAudioSource push 模式返回的 IO 设备。
    QAudioFormat  m_audioCaptureFormat;         ///< 实际使用的采集格式（富措备支持的格式）。
    QByteArray    m_audioPcmBuffer;             ///< PCM 采集缓冲区，凸凹 960 采样 Opus 帧。
    QString       m_micId;                     ///< 当前麦克风设备 ID。

    // 音频播放。
    std::unique_ptr<QAudioSink> m_audioSink;     ///< Qt 音频播放设备。
    QIODevice                  *m_audioIO = nullptr; ///< 音频播放 IO 设备。

    // 音量控制（主线程写、音频采集线程读，需原子操作）。
    std::atomic<int> m_captureVolume{100};  ///< 采集音量（0~100）。
    std::atomic<int> m_playbackVolume{100}; ///< 播放音量（0~100）。

    // 帧率节流：记录各路流上次处理帧的时间戳（后端线程读写，需原子操作）。
    std::atomic<qint64> m_lastCameraFrameMs{0}; ///< 摄像头流上次处理帧的毫秒时间戳。
    std::atomic<qint64> m_lastScreenFrameMs{0}; ///< 屏幕共享流上次处理帧的毫秒时间戳。

    // 关键帧请求辅助状态。
    QHash<QString, int>    m_decodeFailCount;    ///< decoderKey → 连续解码失败帧数（阈值 8 帧触发请求）。
    QHash<QString, qint64> m_lastKeyframeReqMs; ///< userId → 上次发送关键帧请求的毫秒时间戳（2 秒冷却）。
};
