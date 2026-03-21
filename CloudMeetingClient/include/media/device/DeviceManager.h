#pragma once
#include <QString>
#include <QList>

struct DeviceInfo
{
    QString id;
    QString name;
};

class DeviceManager
{
public:
    DeviceManager();
    void enumerateDevices();
    QList<DeviceInfo> cameras()    const;
    QList<DeviceInfo> microphones() const;
    void switchCamera(const QString &id);
    void switchMicrophone(const QString &id);

private:
    QList<DeviceInfo> m_cameras;
    QList<DeviceInfo> m_microphones;
    QString          m_currentCameraId;
    QString          m_currentMicId;
};
