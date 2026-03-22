#pragma once
#include "media/capture/IVideoCaptureStrategy.h"
#include <QString>

/**
 * @file CameraCaptureStrategy.h
 * @brief 声明摄像头采集策略实现。
 */

/**
 * @class CameraCaptureStrategy
 * @brief 封装基于摄像头设备的视频采集策略。
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
    QString m_deviceId;      ///< 当前使用的摄像头设备标识。
    bool    m_running = false; ///< 采集任务运行状态。
};
