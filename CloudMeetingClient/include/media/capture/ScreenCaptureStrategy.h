#pragma once
#include "media/capture/IVideoCaptureStrategy.h"
#include <QMediaCaptureSession>
#include <QScreenCapture>
#include <QVideoSink>
#include <QMetaObject>
#include <atomic>

/**
 * @file ScreenCaptureStrategy.h
 * @brief 声明屏幕共享采集策略实现。
 */

/**
 * @class ScreenCaptureStrategy
 * @brief 封装基于 Qt Multimedia QScreenCapture 的全屏采集策略。
 *
 * Qt Multimedia 对象全部在主线程构造与析构，start()/stop() 均从主线程调用，
 * 帧回调通过 Qt::DirectConnection 在后端线程执行（仅做图像转换，不访问 Qt 对象），
 * 彻底避免跨线程停止时的死锁问题。
 */
class ScreenCaptureStrategy : public IVideoCaptureStrategy
{
public:
    /**
     * @brief 构造屏幕采集策略。
     */
    ScreenCaptureStrategy();

    /**
     * @brief 析构并确保采集已停止。
     */
    ~ScreenCaptureStrategy() override;

    /**
     * @brief 启动屏幕采集。
     * @return 若成功启动采集则返回 `true`，否则返回 `false`。
     */
    bool start() override;

    /**
     * @brief 停止屏幕采集。
     */
    void stop() override;

    /**
     * @brief 查询屏幕采集是否正在运行。
     * @return 若采集任务处于运行状态则返回 `true`，否则返回 `false`。
     */
    bool isRunning() const override;

private:
    /**
     * @brief 处理后端线程投递的视频帧（转换、缩放后调用回调）。
     * @param[in] frame 原始视频帧。
     */
    void onVideoFrame(const QVideoFrame &frame);

    std::atomic<bool>       m_running{false};   ///< 采集运行状态（原子，线程安全）。
    QMediaCaptureSession    m_session;           ///< 媒体采集会话。
    QScreenCapture          m_screenCapture;     ///< 屏幕捕获对象（主线程生命周期）。
    QVideoSink              m_videoSink;         ///< 接收原始视频帧。
    QMetaObject::Connection m_connection;        ///< videoFrameChanged 连接句柄，用于 stop 时断开。
};
