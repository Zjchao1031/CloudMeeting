#pragma once
#include "protocol/SignalType.h"
#include <string>

struct SignalPacket
{
    SignalType  type;
    uint32_t    payloadLen = 0;
    std::string payload;   // UTF-8 JSON 字符串
};
