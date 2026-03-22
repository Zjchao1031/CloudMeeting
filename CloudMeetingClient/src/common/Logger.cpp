/**
 * @file Logger.cpp
 * @brief 实现日志输出工具类。
 */
#include "common/Logger.h"
#include <QDebug>

void Logger::info(const QString &msg)  { qInfo()    << "[INFO] " << msg; }
void Logger::warn(const QString &msg)  { qWarning() << "[WARN] " << msg; }
void Logger::error(const QString &msg) { qCritical()<< "[ERROR]" << msg; }
