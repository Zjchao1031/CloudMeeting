/**
 * @file CameraCaptureStrategy.cpp
 * @brief 实现摄像头采集策略（Qt Multimedia，主线程对象 + 后端线程帧回调）。
 */
#include "media/capture/CameraCaptureStrategy.h"
#include "common/Constants.h"
#include "common/Logger.h"
#include <QCameraDevice>
#include <QMediaDevices>
#include <QVideoFrame>
#include <QImage>

CameraCaptureStrategy::CameraCaptureStrategy(const QString &deviceId)
    : m_deviceId(deviceId)
{}

CameraCaptureStrategy::~CameraCaptureStrategy()
{
    stop();
}

void CameraCaptureStrategy::setDeviceId(const QString &deviceId)
{
    m_deviceId = deviceId;
}

bool CameraCaptureStrategy::start()
{
    if (m_running) return true;

    // 根据设备描述字符串查找摄像头，找不到则使用系统默认输入。
    QCameraDevice selectedDevice = QMediaDevices::defaultVideoInput();
    if (!m_deviceId.isEmpty()) {
        for (const QCameraDevice &dev : QMediaDevices::videoInputs()) {
            if (dev.description() == m_deviceId) {
                selectedDevice = dev;
                break;
            }
        }
    }

    // 组装采集管线：会话 → 摄像头 → 视频输出槽。
    m_camera.setCameraDevice(selectedDevice);
    m_session.setCamera(&m_camera);
    m_session.setVideoOutput(&m_videoSink);

    // 无 context 的 functor 连接默认在发射线程直接调用（等同 DirectConnection），
    // videoFrameChanged 在 Qt 后端线程触发，onVideoFrame 在该线程执行，不阻塞主线程。
    m_connection = QObject::connect(
        &m_videoSink, &QVideoSink::videoFrameChanged,
        [this](const QVideoFrame &frame) { onVideoFrame(frame); });

    m_running = true;
    m_camera.start();
    Logger::info(QStringLiteral("[CameraCapture] 已启动: %1").arg(selectedDevice.description()));
    return true;
}

void CameraCaptureStrategy::stop()
{
    if (!m_running) return;
    m_running = false;

    // 先断开信号，确保后端线程不再触发新帧回调。
    QObject::disconnect(m_connection);

    // 在主线程直接调用 stop()，Qt Multimedia 后端自行完成清理，无死锁风险。
    m_camera.stop();
    Logger::info(QStringLiteral("[CameraCapture] 已停止"));
}

bool CameraCaptureStrategy::isRunning() const
{
    return m_running;
}

void CameraCaptureStrategy::onVideoFrame(const QVideoFrame &frame)
{
    // 此函数在 Qt Multimedia 后端线程执行，禁止访问主线程 Qt 对象。
    if (!m_running || !m_frameCallback || !frame.isValid()) return;

    QImage img = frame.toImage();
    if (img.isNull()) return;

    // 缩放到目标分辨率并统一为 RGB32。
    if (img.width() != Constants::VIDEO_WIDTH || img.height() != Constants::VIDEO_HEIGHT)
        img = img.scaled(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT,
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    if (img.format() != QImage::Format_RGB32)
        img = img.convertToFormat(QImage::Format_RGB32);

    m_frameCallback(img);
}
