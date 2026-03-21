#pragma once
#include "protocol/SignalPacket.h"
#include <string>
#include <vector>

class SignalCodec
{
public:
    // 编码：type(1) + length(4,大端) + payload(N)
    static std::vector<uint8_t> encode(SignalType type, const std::string &jsonPayload);
    // 解码：从 buf 中尝试解出一个完整包，成功返回 true，consumed 为消耗字节数
    static bool decode(const uint8_t *buf, size_t len, SignalPacket &pkt, size_t &consumed);
};
