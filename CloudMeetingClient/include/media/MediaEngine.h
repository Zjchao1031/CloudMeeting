#pragma once
#include <memory>

/**
 * @file MediaEngine.h
 * @brief 声明媒体能力调度引擎。
 */

class IVideoCaptureStrategy;

/**
 * @class MediaEngine
 * @brief 负责统一管理摄像头采集、屏幕共享与音量控制。
 */
class MediaEngine
{
public:
    /**
     * @brief 构造媒体引擎。
     */
    MediaEngine();

    /**
     * @brief 析构媒体引擎并停止正在运行的媒体采集。
     */
    ~MediaEngine();

    /**
     * @brief 启动摄像头采集。
     */
    void startCameraCapture();

    /**
     * @brief 停止摄像头采集。
     */
    void stopCameraCapture();

    /**
     * @brief 启动屏幕共享采集。
     */
    void startScreenShare();

    /**
     * @brief 停止屏幕共享采集。
     */
    void stopScreenShare();

    /**
     * @brief 设置采集音量。
     * @param[in] value 采集音量值，范围为 0 到 100。
     */
    void setCaptureVolume(int value);  // 0~100

    /**
     * @brief 设置播放音量。
     * @param[in] value 播放音量值，范围为 0 到 100。
     */
    void setPlaybackVolume(int value); // 0~100

private:
    std::unique_ptr<IVideoCaptureStrategy> m_cameraCaptureStrategy; ///< 摄像头采集策略对象。
    std::unique_ptr<IVideoCaptureStrategy> m_screenCaptureStrategy; ///< 屏幕共享采集策略对象。
};
