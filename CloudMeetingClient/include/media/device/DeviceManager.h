#pragma once
#include <QString>
#include <QList>

/**
 * @file DeviceManager.h
 * @brief 声明媒体设备管理器。
 */

/**
 * @struct DeviceInfo
 * @brief 描述单个媒体设备的标识与名称。
 */
struct DeviceInfo
{
    QString id;   ///< 设备唯一标识。
    QString name; ///< 设备显示名称。
};

/**
 * @class DeviceManager
 * @brief 负责枚举并切换音视频输入设备。
 */
class DeviceManager
{
public:
    /**
     * @brief 构造设备管理器。
     */
    DeviceManager();

    /**
     * @brief 枚举当前系统可用的音视频设备。
     */
    void enumerateDevices();

    /**
     * @brief 获取当前可用摄像头列表。
     * @return 摄像头设备信息列表。
     */
    QList<DeviceInfo> cameras() const;

    /**
     * @brief 获取当前可用麦克风列表。
     * @return 麦克风设备信息列表。
     */
    QList<DeviceInfo> microphones() const;

    /**
     * @brief 切换当前摄像头设备。
     * @param[in] id 目标摄像头设备标识。
     */
    void switchCamera(const QString &id);

    /**
     * @brief 切换当前麦克风设备。
     * @param[in] id 目标麦克风设备标识。
     */
    void switchMicrophone(const QString &id);

private:
    QList<DeviceInfo> m_cameras;      ///< 当前可用摄像头列表。
    QList<DeviceInfo> m_microphones;  ///< 当前可用麦克风列表。
    QString           m_currentCameraId; ///< 当前选中的摄像头标识。
    QString           m_currentMicId;    ///< 当前选中的麦克风标识。
};
