#pragma once

namespace Constants
{
    // 网络端口
    constexpr quint16 TCP_SIGNAL_PORT   = 9000;
    constexpr quint16 UDP_AUDIO_UP_PORT = 9001;
    constexpr quint16 UDP_VIDEO_UP_PORT = 9002;
    constexpr quint16 UDP_AUDIO_DN_PORT = 9003;
    constexpr quint16 UDP_VIDEO_DN_PORT = 9004;

    // 心跳与重连
    constexpr int HEARTBEAT_INTERVAL_MS  = 5000;
    constexpr int RECONNECT_INTERVAL_MS  = 2000;
    constexpr int RECONNECT_MAX_ATTEMPTS = 5;
    constexpr int HEARTBEAT_TIMEOUT_MS   = 10000;

    // 媒体参数
    constexpr int VIDEO_WIDTH   = 1280;
    constexpr int VIDEO_HEIGHT  = 720;
    constexpr int VIDEO_FPS     = 24;
    constexpr int AUDIO_SAMPLE_RATE = 48000;
    constexpr int AUDIO_CHANNELS    = 1;

    // 聊天
    constexpr int CHAT_MAX_BYTES = 1024;
} // namespace Constants
