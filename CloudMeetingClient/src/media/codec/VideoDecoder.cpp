/**
 * @file VideoDecoder.cpp
 * @brief 实现视频解码器封装类。
 */
#include "media/codec/VideoDecoder.h"

VideoDecoder::VideoDecoder() {}
VideoDecoder::~VideoDecoder() { close(); }

bool VideoDecoder::open()
{
    m_opened = true;
    return true;
}

void VideoDecoder::close() { m_opened = false; }

QImage VideoDecoder::decode(const QByteArray &h264Data)
{
    Q_UNUSED(h264Data)
    return {}; // 待实现：FFmpeg H.264 解码。
}
