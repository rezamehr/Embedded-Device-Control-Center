
#include "Logger.h"

namespace edcc {

Logger& Logger::instance()
{
    // Meyers' Singleton – thread-safe in C++11 and later
    static Logger instance;
    return instance;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
{
}

void Logger::log(LogLevel level, const QString &message, const QString &source)
{
    // Lock to make logging safe when called from different threads
    QMutexLocker locker(&m_mutex);

    const QString timestamp = QDateTime::currentDateTime()
                                  .toString("hh:mm:ss.zzz");

    emit logMessage(level, timestamp, source, message);
}

void Logger::debug(const QString &message, const QString &source)
{
    log(LogLevel::Debug, message, source);
}

void Logger::info(const QString &message, const QString &source)
{
    log(LogLevel::Info, message, source);
}

void Logger::warning(const QString &message, const QString &source)
{
    log(LogLevel::Warning, message, source);
}

void Logger::error(const QString &message, const QString &source)
{
    log(LogLevel::Error, message, source);
}

void Logger::critical(const QString &message, const QString &source)
{
    log(LogLevel::Critical, message, source);
}

} // namespace edcc
