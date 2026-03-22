#pragma once
#include <QByteArray>
#include <QImage>

/**
 * @file VideoDecoder.h
 * @brief 声明 H.264 视频解码器封装类。
 */

/**
 * @class VideoDecoder
 * @brief 封装视频解码器的打开、关闭与帧解码流程。
 */
class VideoDecoder
{
public:
    /**
     * @brief 构造视频解码器。
     */
    VideoDecoder();

    /**
     * @brief 析构视频解码器并释放资源。
     */
    ~VideoDecoder();

    /**
     * @brief 打开视频解码器。
     * @return 若解码器成功初始化则返回 `true`，否则返回 `false`。
     */
    bool open();

    /**
     * @brief 关闭视频解码器。
     */
    void close();

    /**
     * @brief 将 H.264 数据解码为图像帧。
     * @param[in] h264Data 待解码的 H.264 NALU 数据。
     * @return 解码得到的视频图像帧。
     */
    QImage decode(const QByteArray &h264Data);

private:
    bool m_opened = false; ///< 解码器是否已成功打开。
};
