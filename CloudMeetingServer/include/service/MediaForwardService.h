#pragma once
#include <string>
#include <cstdint>

class MediaForwardService
{
public:
    static MediaForwardService &instance();

    // 注入下行发送 fd（由 UdpMediaServer 在绑定后调用）
    void setAudioDnFd(int fd);
    void setVideoDnFd(int fd);

    // 转发音频包：按 userId 查找所在房间，转发给其他成员
    void forwardAudio(uint32_t userId, const uint8_t *data, size_t len,
                      const struct sockaddr_in &srcAddr);
    // 转发视频包
    void forwardVideo(uint32_t userId, const uint8_t *data, size_t len,
                      const struct sockaddr_in &srcAddr);

private:
    MediaForwardService() = default;
    int m_audioDnFd = -1;
    int m_videoDnFd = -1;
};
