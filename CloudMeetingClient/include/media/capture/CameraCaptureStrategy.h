#pragma once
#include "media/capture/IVideoCaptureStrategy.h"
#include <QString>

class CameraCaptureStrategy : public IVideoCaptureStrategy
{
public:
    explicit CameraCaptureStrategy(const QString &deviceId = {});
    bool start()    override;
    void stop()     override;
    bool isRunning() const override;

private:
    QString m_deviceId;
    bool    m_running = false;
};
