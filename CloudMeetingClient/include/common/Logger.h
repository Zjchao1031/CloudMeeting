#pragma once
#include <QString>

/**
 * @file Logger.h
 * @brief 声明日志输出工具类。
 */

/**
 * @class Logger
 * @brief 提供统一的日志输出接口。
 */
class Logger
{
public:
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
};
