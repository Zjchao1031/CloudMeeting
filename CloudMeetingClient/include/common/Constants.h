#pragma once

/**
 * @file Constants.h
 * @brief 定义客户端通用常量。
 */

/**
 * @namespace Constants
 * @brief 提供网络、媒体与聊天模块使用的静态常量。
 */
namespace Constants
{
    /**
     * @name 心跳与重连
     * @{
     */
    constexpr int CONNECT_TIMEOUT_MS    = 5000;  ///< 首次连接超时时间（毫秒）。
    constexpr int HEARTBEAT_INTERVAL_MS = 5000;  ///< 心跳发送间隔，单位：毫秒。
    constexpr int RECONNECT_INTERVAL_MS  = 2000;  ///< 自动重连间隔，单位：毫秒。
    constexpr int RECONNECT_MAX_ATTEMPTS = 5;     ///< 最大重连尝试次数。
    constexpr int HEARTBEAT_TIMEOUT_MS   = 10000; ///< 心跳超时时间，单位：毫秒。
    /**
     * @}
     */

    /**
     * @name 媒体参数
     * @{
     */
    constexpr int VIDEO_WIDTH       = 1280;   ///< 默认视频宽度，单位：像素。
    constexpr int VIDEO_HEIGHT      = 720;    ///< 默认视频高度，单位：像素。
    constexpr int VIDEO_FPS         = 24;     ///< 默认视频帧率。
    constexpr int AUDIO_SAMPLE_RATE = 48000;  ///< 默认音频采样率。
    constexpr int AUDIO_CHANNELS    = 1;      ///< 默认音频通道数。
    constexpr int VIDEO_KEYFRAME_FAIL_THRESHOLD = 8; ///< 连续解码失败帧数阈值，达到后向发送方请求关键帧。
    /**
     * @}
     */

    /**
     * @name 聊天
     * @{
     */
    constexpr int CHAT_MAX_BYTES = 1024;      ///< 单条聊天消息允许的最大 UTF-8 字节数。
    /**
     * @}
     */
} // namespace Constants
