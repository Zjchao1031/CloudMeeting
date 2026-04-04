#pragma once
#include <QByteArray>
#include <QImage>

struct AVCodecContext;
struct AVCodecParserContext;
struct SwsContext;
struct AVFrame;
struct AVPacket;

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
     * @return 解码得到的视频图像帧；若无完整帧输出则返回空 QImage。
     */
    QImage decode(const QByteArray &h264Data);

    /**
     * @brief 刷新解码器内部状态（清空 DPB），在关键帧丢失后请求新 IDR 前调用。
     */
    void flush();

private:
    AVCodecContext       *m_codecCtx  = nullptr; ///< FFmpeg H.264 解码器上下文。
    AVCodecParserContext *m_parser    = nullptr; ///< H.264 码流解析器。
    SwsContext           *m_swsCtx    = nullptr; ///< YUV→RGB32 转换上下文。
    AVFrame              *m_frame     = nullptr; ///< 解码输出帧。
    AVFrame              *m_rgbFrame  = nullptr; ///< RGB32 转换帧。
    AVPacket             *m_pkt       = nullptr; ///< 解码输入数据包。
    uint8_t              *m_rgbBuf    = nullptr; ///< RGB32 帧缓冲区。
    int                   m_swsW      = 0;       ///< 当前 swscale 源宽度。
    int                   m_swsH      = 0;       ///< 当前 swscale 源高度。
    bool                  m_opened    = false;   ///< 解码器是否已成功打开。
};
