#pragma once
#include "media/capture/IVideoCaptureStrategy.h"

/**
 * @file ScreenCaptureStrategy.h
 * @brief 声明屏幕共享采集策略实现。
 */

/**
 * @class ScreenCaptureStrategy
 * @brief 封装基于屏幕内容的视频采集策略。
 */
class ScreenCaptureStrategy : public IVideoCaptureStrategy
{
public:
    /**
     * @brief 构造屏幕采集策略。
     */
    ScreenCaptureStrategy();

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
    bool m_running = false; ///< 屏幕采集任务运行状态。
};
