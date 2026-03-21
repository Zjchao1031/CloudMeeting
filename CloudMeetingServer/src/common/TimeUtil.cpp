#include "common/TimeUtil.h"
#include <chrono>

namespace TimeUtil
{
    int64_t nowSeconds()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    bool isTimeout(int64_t ts, int thresholdSec)
    {
        return (nowSeconds() - ts) >= thresholdSec;
    }
} // namespace TimeUtil
