#pragma once

namespace Constants
{
    constexpr uint16_t TCP_SIGNAL_PORT    = 9000;
    constexpr uint16_t UDP_AUDIO_UP_PORT  = 9001;
    constexpr uint16_t UDP_VIDEO_UP_PORT  = 9002;
    constexpr uint16_t UDP_AUDIO_DN_PORT  = 9003;
    constexpr uint16_t UDP_VIDEO_DN_PORT  = 9004;

    constexpr int HEARTBEAT_TIMEOUT_SEC   = 15; // 超过此时间未收心跳则断线
    constexpr int EPOLL_MAX_EVENTS        = 1024;
    constexpr int ROOM_ID_LENGTH          = 6;
} // namespace Constants
