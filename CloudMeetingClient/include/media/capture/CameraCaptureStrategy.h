#pragma once
#include "media/capture/IVideoCaptureStrategy.h"
#include <QString>
#include <QThread>
#include <QMutex>
#include <atomic>

struct AVFormatContext;
struct AVCodecContext;
struct SwsContext;

/**
 * @file CameraCaptureStrategy.h
 * @brief 声明摄像头采集策略实现。
 */

/**
 * @class CameraCaptureStrategy
 * @brief 封装基于 FFmpeg dshow 的摄像头视频采集策略，运行于独立线程。
 */
class CameraCaptureStrategy : public IVideoCaptureStrategy
{
public:
    /**
     * @brief 构造摄像头采集策略。
     * @param[in] deviceId 目标摄像头设备标识；为空时表示使用默认设备。
     */
    explicit CameraCaptureStrategy(const QString &deviceId = {});

    /**
     * @brief 析构并确保采集线程已停止。
     */
    ~CameraCaptureStrategy() override;

    /**
     * @brief 设置采集使用的设备 ID。
     * @param[in] deviceId 摄像头设备标识。
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
     * @brief 采集线程主循环：打开设备、循环读帧、转换为 QImage 并回调。
     */
    void captureLoop();

    QString            m_deviceId;                ///< 当前使用的摄像头设备标识。
    mutable QMutex     m_deviceIdMutex;            ///< 保护 m_deviceId 读写的互斥量。
    std::atomic<bool>  m_running{false};           ///< 采集任务运行状态（线程安全）。
    QThread            m_thread;                   ///< 采集工作线程。
};
