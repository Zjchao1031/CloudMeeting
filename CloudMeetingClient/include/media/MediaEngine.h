#pragma once
#include <QObject>
#include <QImage>
#include <QByteArray>
#include <QThread>
#include <QAudioSink>
#include <QMediaDevices>
#include <QString>
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
     */
    void forceVideoKeyFrame();

    /**
     * @brief 设置本地用户 ID，用于标识发送的媒体数据包。
     * @param[in] userId 本地用户 ID。
     */
    void setLocalUserId(const QString &userId);

    /**
     * @brief 移除指定远端用户的视频解码器，在成员离开时调用以释放资源。
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
     */
    void remoteVideoFrame(const QString &userId, const QImage &frame);

    /**
     * @brief 本地预览视频帧时发出。
     * @param[in] frame 本地采集的视频图像。
     */
    void localVideoFrame(const QImage &frame);

private slots:
    /**
     * @brief 处理从网络接收到的远端视频数据。
     * @param[in] userId 发送方用户 ID。
     * @param[in] h264Data H.264 编码数据。
     */
    void onRemoteVideoData(const QString &userId, const QByteArray &h264Data);

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
     * @brief 音频采集线程主循环。
     */
    void audioCaptureLoop();

    NetworkFacade *m_network = nullptr;  ///< 绑定的网络门面。
    QString        m_localUserId;        ///< 本地用户 ID。

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

    // 音频采集线程。
    QThread           m_audioCaptureThread; ///< 麦克风采集工作线程。
    std::atomic<bool> m_audioRunning{false}; ///< 音频采集运行标志。
    QString           m_micId;               ///< 当前麦克风设备 ID。

    // 音频播放。
    std::unique_ptr<QAudioSink> m_audioSink;     ///< Qt 音频播放设备。
    QIODevice                  *m_audioIO = nullptr; ///< 音频播放 IO 设备。

    // 音量控制（主线程写、音频采集线程读，需原子操作）。
    std::atomic<int> m_captureVolume{100};  ///< 采集音量（0~100）。
    std::atomic<int> m_playbackVolume{100}; ///< 播放音量（0~100）。
};
