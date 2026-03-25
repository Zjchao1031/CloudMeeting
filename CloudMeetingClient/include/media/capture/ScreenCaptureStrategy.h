#pragma once
#include "media/capture/IVideoCaptureStrategy.h"
#include <QThread>
#include <atomic>

/**
 * @file ScreenCaptureStrategy.h
 * @brief 声明屏幕共享采集策略实现。
 */

/**
 * @class ScreenCaptureStrategy
 * @brief 封装基于 FFmpeg gdigrab 的全屏截图采集策略，运行于独立线程。
 */
class ScreenCaptureStrategy : public IVideoCaptureStrategy
{
public:
    /**
     * @brief 构造屏幕采集策略。
     */
    ScreenCaptureStrategy();

    /**
     * @brief 析构并确保采集线程已停止。
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
     * @brief 采集线程主循环：使用 gdigrab 捕获全屏并转换为 QImage。
     */
    void captureLoop();

    std::atomic<bool> m_running{false}; ///< 屏幕采集任务运行状态（线程安全）。
    QThread           m_thread;          ///< 采集工作线程。
};
