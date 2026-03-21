#pragma once
#include <string>
#include <cstdint>

class MediaForwardService
{
public:
    static MediaForwardService &instance();

    // 转发音频包：按 userId 查找所在房间，转发给其他成员
    void forwardAudio(uint32_t userId, const uint8_t *data, size_t len);
    // 转发视频包
    void forwardVideo(uint32_t userId, const uint8_t *data, size_t len);

private:
    MediaForwardService() = default;
};
