#pragma once
#include <string>
#include <cstdint>

namespace IdGenerator
{
    // 生成指定长度的随机数字房间号
    std::string generateRoomId(int length = 6);
    // 生成唯一用户 ID（前缀 + 随机串）
    std::string generateUserId();
    // 生成自增数字用户 ID（用于 UDP 包头 uint32_t 字段）
    uint32_t generateNumericUserId();
} // namespace IdGenerator
