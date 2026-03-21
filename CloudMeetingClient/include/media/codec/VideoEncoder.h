#pragma once
#include <QByteArray>
#include <QImage>

// 封装 FFmpeg H.264 编码逻辑
class VideoEncoder
{
public:
    VideoEncoder();
    ~VideoEncoder();

    bool open(int width, int height, int fps);
    void close();
    // 输入 QImage（RGB32），输出 H.264 NALU 数据
    QByteArray encode(const QImage &frame);

private:
    bool m_opened = false;
};
