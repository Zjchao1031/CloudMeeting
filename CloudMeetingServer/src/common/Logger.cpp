#include "common/Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

Logger &Logger::instance()
{
    static Logger s;
    return s;
}

void Logger::log(const std::string &type, const std::string &msg)
{
    auto now = std::chrono::system_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                   now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << '[' << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count()
        << ']' << type << " : " << msg << '\n';

    std::lock_guard<std::mutex> lk(m_mutex);
    std::cout << oss.str();
}

void Logger::info(const std::string &msg)  { instance().log("INFO",  msg); }
void Logger::warn(const std::string &msg)  { instance().log("WARN",  msg); }
void Logger::error(const std::string &msg) { instance().log("ERROR", msg); }
