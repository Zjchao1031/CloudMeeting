#include "media/codec/VideoEncoder.h"

VideoEncoder::VideoEncoder() {}
VideoEncoder::~VideoEncoder() { close(); }

bool VideoEncoder::open(int width, int height, int fps)
{
    Q_UNUSED(width) Q_UNUSED(height) Q_UNUSED(fps)
    m_opened = true;
    return true;
}

void VideoEncoder::close() { m_opened = false; }

QByteArray VideoEncoder::encode(const QImage &frame)
{
    Q_UNUSED(frame)
    return {}; // TODO: FFmpeg H.264 编码
}
