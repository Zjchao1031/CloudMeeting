#pragma once
#include <QByteArray>
#include <QImage>

/**
 * @file VideoEncoder.h
 * @brief 声明 H.264 视频编码器封装类。
 */

/**
 * @class VideoEncoder
 * @brief 封装视频编码器的打开、关闭与帧编码流程。
 */
class VideoEncoder
{
public:
    /**
     * @brief 构造视频编码器。
     */
    VideoEncoder();

    /**
     * @brief 析构视频编码器并释放资源。
     */
    ~VideoEncoder();

    /**
     * @brief 打开视频编码器。
     * @param[in] width 输入图像宽度。
     * @param[in] height 输入图像高度。
     * @param[in] fps 输入图像帧率。
     * @return 若编码器成功初始化则返回 `true`，否则返回 `false`。
     */
    bool open(int width, int height, int fps);

    /**
     * @brief 关闭视频编码器。
     */
    void close();

    /**
     * @brief 将图像帧编码为 H.264 数据。
     * @param[in] frame 待编码的视频图像帧。
     * @return 编码得到的 H.264 NALU 数据。
     */
    QByteArray encode(const QImage &frame);

private:
    bool m_opened = false; ///< 编码器是否已成功打开。
};
