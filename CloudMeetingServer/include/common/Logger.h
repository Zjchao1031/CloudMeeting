#pragma once
#include <string>
#include <mutex>

class Logger
{
public:
    static Logger &instance();

    static void info(const std::string &msg);
    static void warn(const std::string &msg);
    static void error(const std::string &msg);

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    void log(const std::string &type, const std::string &msg);

    std::mutex m_mutex;
};
