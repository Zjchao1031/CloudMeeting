/**
 * @file DeviceManager.cpp
 * @brief 实现媒体设备管理器。
 */
#include "media/device/DeviceManager.h"
#include "common/Logger.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <QSet>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
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
                // dshow 的 video=<name> URL 需要友好名称（device_description），
                // 而非 PnP 内部路径（device_name），否则 avformat_open_input 将失败。
                info.name = QString::fromUtf8(dev->device_description);
                info.id   = info.name.isEmpty()
                            ? QString::fromUtf8(dev->device_name)
                            : info.name;
                if (info.name.isEmpty()) info.name = info.id;
                Logger::info(QStringLiteral("[DeviceManager] 摄像头[%1] name=%2  desc=%3")
                             .arg(i)
                             .arg(QString::fromUtf8(dev->device_name))
                             .arg(QString::fromUtf8(dev->device_description)));
                m_cameras.append(info);
            }
            avdevice_free_list_devices(&devList);
        }
    }

    // 枚举音频设备（麦克风）：使用 Qt QMediaDevices 走 Windows Core Audio 端点，
    // 避免 dshow avdevice_list_input_sources("audio") 错误地将视频设备混入列表。
    {
        const QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
        for (int i = 0; i < audioInputs.size(); ++i) {
            const QAudioDevice &dev = audioInputs.at(i);
            DeviceInfo info;
            info.name = dev.description();
            info.id   = info.name.isEmpty()
                        ? QString::fromUtf8(dev.id())
                        : info.name;
            if (info.name.isEmpty()) info.name = info.id;
            Logger::info(QStringLiteral("[DeviceManager] 麦克风[%1] id=%2  desc=%3")
                         .arg(i)
                         .arg(QString::fromUtf8(dev.id()))
                         .arg(dev.description()));
            m_microphones.append(info);
        }
    }

    // 默认选中第一个摄像头（仅首次）。
    if (!m_cameras.isEmpty() && m_currentCameraId.isEmpty())
        m_currentCameraId = m_cameras.first().id;

    // 每次枚举后重新验证麦克风选择：
    // 若当前选中的麦克风已不在新列表中，或与摄像头同名，则重新选择。
    if (!m_microphones.isEmpty()) {
        QSet<QString> cameraIds;
        for (const DeviceInfo &cam : m_cameras)
            cameraIds.insert(cam.id);

        // 检查当前选中的麦克风是否仍有效（在列表中且不与摄像头重名）。
        bool currentValid = false;
        if (!m_currentMicId.isEmpty() && !cameraIds.contains(m_currentMicId)) {
            for (const DeviceInfo &mic : m_microphones) {
                if (mic.id == m_currentMicId) { currentValid = true; break; }
            }
        }

        if (!currentValid) {
            m_currentMicId.clear();
            for (const DeviceInfo &mic : m_microphones) {
                if (!cameraIds.contains(mic.id)) {
                    m_currentMicId = mic.id;
                    break;
                }
            }
            // 若所有麦克风都与摄像头重名，则退回第一个（总比空值好）。
            if (m_currentMicId.isEmpty())
                m_currentMicId = m_microphones.first().id;
        }
    }

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
