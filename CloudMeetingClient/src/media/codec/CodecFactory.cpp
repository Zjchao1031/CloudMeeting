/**
 * @file CodecFactory.cpp
 * @brief 实现媒体编解码器工厂类。
 */
#include "media/codec/CodecFactory.h"
#include "media/codec/VideoEncoder.h"
#include "media/codec/VideoDecoder.h"
#include "media/codec/AudioEncoder.h"
#include "media/codec/AudioDecoder.h"

std::unique_ptr<VideoEncoder> CodecFactory::createVideoEncoder()
{
    return std::make_unique<VideoEncoder>();
}

std::unique_ptr<VideoDecoder> CodecFactory::createVideoDecoder()
{
    return std::make_unique<VideoDecoder>();
}

std::unique_ptr<AudioEncoder> CodecFactory::createAudioEncoder()
{
    return std::make_unique<AudioEncoder>();
}

std::unique_ptr<AudioDecoder> CodecFactory::createAudioDecoder()
{
    return std::make_unique<AudioDecoder>();
}
