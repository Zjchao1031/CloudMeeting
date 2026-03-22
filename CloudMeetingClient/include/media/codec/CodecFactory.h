#pragma once
#include <memory>

/**
 * @file CodecFactory.h
 * @brief 声明媒体编解码器工厂类。
 */

class VideoEncoder;
class VideoDecoder;
class AudioEncoder;
class AudioDecoder;

/**
 * @class CodecFactory
 * @brief 负责创建音视频编解码器实例。
 */
class CodecFactory
{
public:
    /**
     * @brief 创建视频编码器实例。
     * @return 新创建的视频编码器智能指针。
     */
    static std::unique_ptr<VideoEncoder> createVideoEncoder();

    /**
     * @brief 创建视频解码器实例。
     * @return 新创建的视频解码器智能指针。
     */
    static std::unique_ptr<VideoDecoder> createVideoDecoder();

    /**
     * @brief 创建音频编码器实例。
     * @return 新创建的音频编码器智能指针。
     */
    static std::unique_ptr<AudioEncoder> createAudioEncoder();

    /**
     * @brief 创建音频解码器实例。
     * @return 新创建的音频解码器智能指针。
     */
    static std::unique_ptr<AudioDecoder> createAudioDecoder();
};
