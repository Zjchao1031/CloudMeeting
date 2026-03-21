#include "media/MediaEngine.h"
#include "media/capture/CameraCaptureStrategy.h"
#include "media/capture/ScreenCaptureStrategy.h"

MediaEngine::MediaEngine()
    : m_cameraCaptureStrategy(std::make_unique<CameraCaptureStrategy>())
    , m_screenCaptureStrategy(std::make_unique<ScreenCaptureStrategy>())
{}

MediaEngine::~MediaEngine()
{
    stopCameraCapture();
    stopScreenShare();
}

void MediaEngine::startCameraCapture()
{
    m_cameraCaptureStrategy->start();
}

void MediaEngine::stopCameraCapture()
{
    m_cameraCaptureStrategy->stop();
}

void MediaEngine::startScreenShare()
{
    m_screenCaptureStrategy->start();
}

void MediaEngine::stopScreenShare()
{
    m_screenCaptureStrategy->stop();
}

void MediaEngine::setCaptureVolume(int value)
{
    Q_UNUSED(value)
}

void MediaEngine::setPlaybackVolume(int value)
{
    Q_UNUSED(value)
}
