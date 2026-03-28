#pragma once
#include "media/capture/IVideoCaptureStrategy.h"
#include <QMediaCaptureSession>
#include <QCamera>
#include <QVideoSink>
#include <QMetaObject>
#include <QString>
#include <atomic>

/**
 * @file CameraCaptureStrategy.h
 * @brief 声明摄像头采集策略实现。
 */

/**
 * @class CameraCaptureStrategy
 * @brief 封装基于 Qt Multimedia QCamera 的摄像头视频采集策略。
 *
 * Qt Multimedia 对象全部在主线程构造与析构，start()/stop() 均从主线程调用，
 * 帧回调通过 Qt::DirectConnection 在后端线程执行（仅做图像转换，不访问 Qt 对象），
 * 彻底避免跨线程停止时的死锁问题。
 */
class CameraCaptureStrategy : public IVideoCaptureStrategy
{
public:
    /**
     * @brief 构造摄像头采集策略。
     * @param[in] deviceId 目标摄像头设备描述字符串；为空时使用系统默认设备。
     */
    explicit CameraCaptureStrategy(const QString &deviceId = {});

    /**
     * @brief 析构并确保采集已停止。
     */
    ~CameraCaptureStrategy() override;

    /**
     * @brief 设置采集使用的设备描述字符串。
     * @param[in] deviceId 摄像头设备描述字符串。
     */
    void setDeviceId(const QString &deviceId);

    /**
     * @brief 启动摄像头采集。
     * @return 若成功启动采集则返回 `true`，否则返回 `false`。
     */
    bool start() override;

    /**
     * @brief 停止摄像头采集。
     */
    void stop() override;

    /**
     * @brief 查询摄像头采集是否正在运行。
     * @return 若采集任务处于运行状态则返回 `true`，否则返回 `false`。
     */
    bool isRunning() const override;

private:
    /**
     * @brief 处理后端线程投递的视频帧（转换、缩放后调用回调）。
     * @param[in] frame 原始视频帧。
     */
    void onVideoFrame(const QVideoFrame &frame);

    QString                 m_deviceId;             ///< 目标摄像头设备描述字符串。
    std::atomic<bool>       m_running{false};        ///< 采集运行状态（原子，线程安全）。
    QMediaCaptureSession    m_session;               ///< 媒体采集会话。
    QCamera                 m_camera;                ///< 摄像头对象（主线程生命周期）。
    QVideoSink              m_videoSink;             ///< 接收原始视频帧。
    QMetaObject::Connection m_connection;            ///< videoFrameChanged 连接句柄，用于 stop 时断开。
};
