#pragma once
#include "media/capture/IVideoCaptureStrategy.h"

class ScreenCaptureStrategy : public IVideoCaptureStrategy
{
public:
    ScreenCaptureStrategy();
    bool start()    override;
    void stop()     override;
    bool isRunning() const override;

private:
    bool m_running = false;
};
