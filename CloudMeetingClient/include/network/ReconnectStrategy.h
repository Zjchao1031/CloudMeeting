#pragma once

struct ReconnectStrategy
{
    int intervalMs   = 2000;  // 重连间隔 ms
    int maxAttempts  = 5;     // 最大重试次数

    bool shouldRetry(int attempts) const { return attempts < maxAttempts; }
};
