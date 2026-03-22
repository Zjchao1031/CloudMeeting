#pragma once

/**
 * @file ReconnectStrategy.h
 * @brief 定义网络重连策略参数。
 */

/**
 * @struct ReconnectStrategy
 * @brief 描述自动重连的间隔与次数限制。
 */
struct ReconnectStrategy
{
    int intervalMs  = 2000; ///< 重连间隔，单位：毫秒。
    int maxAttempts = 5;    ///< 最大重试次数。

    /**
     * @brief 判断在当前尝试次数下是否还应继续重连。
     * @param[in] attempts 当前已经执行的重连次数。
     * @return 若仍允许继续重连则返回 `true`，否则返回 `false`。
     */
    bool shouldRetry(int attempts) const { return attempts < maxAttempts; }
};
