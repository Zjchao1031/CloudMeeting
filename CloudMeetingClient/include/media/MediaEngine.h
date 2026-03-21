#pragma once
#include <memory>

class IVideoCaptureStrategy;
class VideoEncoder;
class AudioEncoder;
class VideoDecoder;
class AudioDecoder;

class MediaEngine
{
public:
    MediaEngine();
    ~MediaEngine();

    void startCameraCapture();
    void stopCameraCapture();
    void startScreenShare();
    void stopScreenShare();
    void setCaptureVolume(int value);  // 0~100
    void setPlaybackVolume(int value); // 0~100

private:
    std::unique_ptr<IVideoCaptureStrategy> m_cameraCaptureStrategy;
    std::unique_ptr<IVideoCaptureStrategy> m_screenCaptureStrategy;
};
