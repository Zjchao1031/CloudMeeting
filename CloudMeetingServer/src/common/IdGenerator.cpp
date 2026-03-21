#include "common/IdGenerator.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace IdGenerator
{
    std::string generateRoomId(int length)
    {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 9);
        std::string id;
        id.reserve(length);
        for (int i = 0; i < length; ++i)
            id += static_cast<char>('0' + dist(rng));
        return id;
    }

    std::string generateUserId()
    {
        static std::mt19937_64 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist;
        std::ostringstream oss;
        oss << "u_" << std::hex << std::setw(12) << std::setfill('0') << dist(rng);
        return oss.str();
    }
} // namespace IdGenerator
