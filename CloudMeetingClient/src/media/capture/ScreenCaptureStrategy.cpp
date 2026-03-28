/**
 * @file ScreenCaptureStrategy.cpp
 * @brief 实现屏幕共享采集策略（Qt Multimedia，主线程对象 + 后端线程帧回调）。
 */
#include "media/capture/ScreenCaptureStrategy.h"
#include "common/Constants.h"
#include "common/Logger.h"
#include <QVideoFrame>
#include <QImage>
#include <QScreen>
#include <QGuiApplication>

ScreenCaptureStrategy::ScreenCaptureStrategy() {}

ScreenCaptureStrategy::~ScreenCaptureStrategy()
{
    stop();
}

bool ScreenCaptureStrategy::start()
{
    if (m_running) return true;

    // 组装采集管线：会话 → 屏幕捕获 → 视频输出槽。
    m_session.setScreenCapture(&m_screenCapture);
    m_session.setVideoOutput(&m_videoSink);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) m_screenCapture.setScreen(screen);

    // 无 context 的 functor 连接默认在发射线程直接调用（等同 DirectConnection），
    // videoFrameChanged 在 Qt 后端线程触发，onVideoFrame 在该线程执行，不阻塞主线程。
    m_connection = QObject::connect(
        &m_videoSink, &QVideoSink::videoFrameChanged,
        [this](const QVideoFrame &frame) { onVideoFrame(frame); });

    m_running = true;
    m_screenCapture.start();
    Logger::info(QStringLiteral("[ScreenCapture] 已启动"));
    return true;
}

void ScreenCaptureStrategy::stop()
{
    if (!m_running) return;
    m_running = false;

    // 先断开信号，确保后端线程不再触发新帧回调。
    QObject::disconnect(m_connection);

    // 在主线程直接调用 stop()，Qt Multimedia 后端自行完成清理，无死锁风险。
    m_screenCapture.stop();
    Logger::info(QStringLiteral("[ScreenCapture] 已停止"));
}

bool ScreenCaptureStrategy::isRunning() const
{
    return m_running;
}

void ScreenCaptureStrategy::onVideoFrame(const QVideoFrame &frame)
{
    // 此函数在 Qt Multimedia 后端线程执行，禁止访问主线程 Qt 对象。
    if (!m_running || !m_frameCallback || !frame.isValid()) return;

    QImage img = frame.toImage();
    if (img.isNull()) return;

    // 缩放到目标分辨率并统一为 RGB32。
    if (img.width() != Constants::VIDEO_WIDTH || img.height() != Constants::VIDEO_HEIGHT)
        img = img.scaled(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT,
                         Qt::IgnoreAspectRatio, Qt::FastTransformation);

    if (img.format() != QImage::Format_RGB32)
        img = img.convertToFormat(QImage::Format_RGB32);

    m_frameCallback(img);
}
