#pragma once
#include <QByteArray>
#include <QImage>

// 封装 FFmpeg H.264 解码逻辑
class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder();

    bool open();
    void close();
    // 输入 H.264 NALU 数据，输出解码帧（QImage）
    QImage decode(const QByteArray &h264Data);

private:
    bool m_opened = false;
};
