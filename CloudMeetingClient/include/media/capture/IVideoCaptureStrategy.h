#pragma once
#include <QByteArray>

// 视频采集策略接口（策略模式抽象基类）
class IVideoCaptureStrategy
{
public:
    virtual ~IVideoCaptureStrategy() = default;
    virtual bool start() = 0;
    virtual void stop()  = 0;
    virtual bool isRunning() const = 0;
};
