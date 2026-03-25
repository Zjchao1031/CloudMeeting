/**
 * @file DeviceManager.cpp
 * @brief 实现媒体设备管理器。
 */
#include "media/device/DeviceManager.h"
#include "common/Logger.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

DeviceManager::DeviceManager()
{
    // 注册所有 avdevice 输入/输出格式。
    avdevice_register_all();
}

void DeviceManager::enumerateDevices()
{
    m_cameras.clear();
    m_microphones.clear();

    // 使用 dshow 枚举 Windows 设备。
    const AVInputFormat *dshowFmt = av_find_input_format("dshow");
    if (!dshowFmt) {
        Logger::warn(QStringLiteral("[DeviceManager] dshow 输入格式不可用"));
        return;
    }

    // 枚举视频设备（摄像头）。
    {
        AVFormatContext *fmtCtx = avformat_alloc_context();
        AVDictionary *opts = nullptr;
        av_dict_set(&opts, "list_devices", "true", 0);
        // avformat_open_input 在 list_devices 模式下会返回错误码，但设备信息
        // 被打印到 AVFormatContext 日志中。我们改用 avdevice_list_input_sources。
        av_dict_free(&opts);
        avformat_free_context(fmtCtx);

        AVDeviceInfoList *devList = nullptr;
        int ret = avdevice_list_input_sources(dshowFmt, "video", nullptr, &devList);
        if (ret >= 0 && devList) {
            for (int i = 0; i < devList->nb_devices; ++i) {
                AVDeviceInfo *dev = devList->devices[i];
                DeviceInfo info;
                info.id   = QString::fromUtf8(dev->device_name);
                info.name = QString::fromUtf8(dev->device_description);
                if (info.name.isEmpty()) info.name = info.id;
                m_cameras.append(info);
            }
            avdevice_free_list_devices(&devList);
        }
    }

    // 枚举音频设备（麦克风）。
    {
        AVDeviceInfoList *devList = nullptr;
        int ret = avdevice_list_input_sources(dshowFmt, "audio", nullptr, &devList);
        if (ret >= 0 && devList) {
            for (int i = 0; i < devList->nb_devices; ++i) {
                AVDeviceInfo *dev = devList->devices[i];
                DeviceInfo info;
                info.id   = QString::fromUtf8(dev->device_name);
                info.name = QString::fromUtf8(dev->device_description);
                if (info.name.isEmpty()) info.name = info.id;
                m_microphones.append(info);
            }
            avdevice_free_list_devices(&devList);
        }
    }

    // 默认选中第一个设备。
    if (!m_cameras.isEmpty() && m_currentCameraId.isEmpty())
        m_currentCameraId = m_cameras.first().id;
    if (!m_microphones.isEmpty() && m_currentMicId.isEmpty())
        m_currentMicId = m_microphones.first().id;

    Logger::info(QStringLiteral("[DeviceManager] 枚举到 %1 个摄像头, %2 个麦克风")
                 .arg(m_cameras.size())
                 .arg(m_microphones.size()));
}

QList<DeviceInfo> DeviceManager::cameras() const      { return m_cameras; }
QList<DeviceInfo> DeviceManager::microphones() const   { return m_microphones; }
QString DeviceManager::currentCameraId() const         { return m_currentCameraId; }
QString DeviceManager::currentMicId() const            { return m_currentMicId; }

void DeviceManager::switchCamera(const QString &id)
{
    m_currentCameraId = id;
}

void DeviceManager::switchMicrophone(const QString &id)
{
    m_currentMicId = id;
}
