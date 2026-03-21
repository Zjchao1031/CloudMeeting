#include "media/device/DeviceManager.h"

DeviceManager::DeviceManager() {}

void DeviceManager::enumerateDevices()
{
    // TODO: 枚举系统摄像头与麦克风设备
}

QList<DeviceInfo> DeviceManager::cameras() const    { return m_cameras; }
QList<DeviceInfo> DeviceManager::microphones() const { return m_microphones; }

void DeviceManager::switchCamera(const QString &id)
{
    m_currentCameraId = id;
}

void DeviceManager::switchMicrophone(const QString &id)
{
    m_currentMicId = id;
}
