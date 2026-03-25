#pragma once
#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>

/**
 * @file Logger.h
 * @brief 声明日志输出工具类。
 */

/**
 * @class Logger
 * @brief 提供统一的日志输出接口，采用单例模式管理日志文件生命周期。
 */
class Logger
{
public:
    /**
     * @brief 获取 Logger 单例。
     * @return Logger 唯一实例的引用。
     */
    static Logger &instance();

    /**
     * @brief 输出信息级日志。
     * @param[in] msg 待输出的日志内容。
     */
    static void info(const QString &msg);

    /**
     * @brief 输出警告级日志。
     * @param[in] msg 待输出的日志内容。
     */
    static void warn(const QString &msg);

    /**
     * @brief 输出错误级日志。
     * @param[in] msg 待输出的日志内容。
     */
    static void error(const QString &msg);

private:
    Logger();
    ~Logger() = default;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    /**
     * @brief 实际写入日志的成员函数。
     * @param[in] type 日志类型字符串。
     * @param[in] msg  日志内容。
     */
    void log(const QString &type, const QString &msg);

    QMutex      m_mutex;
    QFile       m_file;
    QTextStream m_stream;
    bool        m_ready = false;
};
