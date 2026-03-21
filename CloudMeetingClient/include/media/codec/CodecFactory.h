#pragma once
#include <memory>

class VideoEncoder;
class VideoDecoder;
class AudioEncoder;
class AudioDecoder;

class CodecFactory
{
public:
    static std::unique_ptr<VideoEncoder> createVideoEncoder();
    static std::unique_ptr<VideoDecoder> createVideoDecoder();
    static std::unique_ptr<AudioEncoder> createAudioEncoder();
    static std::unique_ptr<AudioDecoder> createAudioDecoder();
};
