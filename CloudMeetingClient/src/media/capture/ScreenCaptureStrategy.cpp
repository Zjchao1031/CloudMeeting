/**
 * @file ScreenCaptureStrategy.cpp
 * @brief 实现屏幕共享采集策略。
 */
#include "media/capture/ScreenCaptureStrategy.h"

ScreenCaptureStrategy::ScreenCaptureStrategy() {}

bool ScreenCaptureStrategy::start()
{
    m_running = true;
    return true;
}

void ScreenCaptureStrategy::stop()
{
    m_running = false;
}

bool ScreenCaptureStrategy::isRunning() const
{
    return m_running;
}
