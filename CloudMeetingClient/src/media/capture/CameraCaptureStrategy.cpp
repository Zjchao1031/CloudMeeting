/**
 * @file CameraCaptureStrategy.cpp
 * @brief 实现摄像头采集策略。
 */
#include "media/capture/CameraCaptureStrategy.h"

CameraCaptureStrategy::CameraCaptureStrategy(const QString &deviceId)
    : m_deviceId(deviceId)
{}

bool CameraCaptureStrategy::start()
{
    m_running = true;
    return true;
}

void CameraCaptureStrategy::stop()
{
    m_running = false;
}

bool CameraCaptureStrategy::isRunning() const
{
    return m_running;
}
