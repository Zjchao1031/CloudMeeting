/**
 * @file Logger.cpp
 * @brief 实现日志输出工具类（单例模式）。
 */
#include <QCoreApplication>
#include <QDateTime>
#include <QMutexLocker>
#include <QStringConverter>
#include "common/Logger.h"

Logger &Logger::instance()
{
    static Logger s;
    return s;
}

Logger::Logger() : m_file(QCoreApplication::applicationDirPath() + "/latest.log")
{
    m_ready = m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    if (m_ready) {
        m_stream.setDevice(&m_file);
        m_stream.setEncoding(QStringConverter::Utf8);
    }
}

void Logger::log(const QString &type, const QString &msg)
{
    if (!m_ready) return;

    QMutexLocker locker(&m_mutex);
    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    m_stream << '[' << timestamp << ']' << type << " : " << msg << '\n';
    m_stream.flush();
}

void Logger::info(const QString &msg)  { instance().log(QStringLiteral("INFO"), msg); }
void Logger::warn(const QString &msg)  { instance().log(QStringLiteral("WARN"), msg); }
void Logger::error(const QString &msg) { instance().log(QStringLiteral("ERROR"), msg); }
