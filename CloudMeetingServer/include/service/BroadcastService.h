#pragma once
#include "protocol/SignalType.h"
#include <string>

class BroadcastService
{
public:
    static BroadcastService &instance();

    // 向指定房间内除 excludeFd 外的所有成员广播
    void broadcastToRoom(const std::string &roomId, SignalType type,
                         const std::string &jsonPayload, int excludeFd = -1);
    // 向单个 fd 发送
    void sendTo(int fd, SignalType type, const std::string &jsonPayload);

private:
    BroadcastService() = default;
};
