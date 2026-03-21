#pragma once
#include <cstdint>

namespace TimeUtil
{
    // 返回当前 Unix 时间戳（秒）
    int64_t nowSeconds();
    // 判断 ts 距现在是否超过 thresholdSec 秒
    bool isTimeout(int64_t ts, int thresholdSec);
} // namespace TimeUtil
