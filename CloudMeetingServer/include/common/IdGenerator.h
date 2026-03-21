#pragma once
#include <string>

namespace IdGenerator
{
    // 生成指定长度的随机数字房间号
    std::string generateRoomId(int length = 6);
    // 生成唯一用户 ID（前缀 + 随机串）
    std::string generateUserId();
} // namespace IdGenerator
