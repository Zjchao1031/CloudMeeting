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
    return {}; // TODO: FFmpeg H.264 解码
}
