#pragma once
#include <QImage>
#include <functional>

/**
 * @file IVideoCaptureStrategy.h
 * @brief 声明视频采集策略抽象接口。
 */

/// 帧回调：采集到一帧 RGB32 图像时调用。
using FrameCallback = std::function<void(const QImage &frame)>;

/**
 * @class IVideoCaptureStrategy
 * @brief 定义视频采集策略模式的统一接口。
 */
class IVideoCaptureStrategy
{
public:
    /**
     * @brief 析构视频采集策略接口。
     */
    virtual ~IVideoCaptureStrategy() = default;

    /**
     * @brief 设置帧到达回调。
     * @param[in] cb 采集到一帧时被调用的回调函数。
     */
    virtual void setFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }

    /**
     * @brief 启动视频采集。
     * @return 若成功启动采集则返回 `true`，否则返回 `false`。
     */
    virtual bool start() = 0;

    /**
     * @brief 停止视频采集。
     */
    virtual void stop()  = 0;

    /**
     * @brief 查询采集任务是否正在运行。
     * @return 若采集任务处于运行状态则返回 `true`，否则返回 `false`。
     */
    virtual bool isRunning() const = 0;

protected:
    FrameCallback m_frameCallback; ///< 帧到达回调函数。
};
