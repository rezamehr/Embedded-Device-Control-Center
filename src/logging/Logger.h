#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QMutex>
#include "core/Types.h"

namespace edcc {

/**
 * @brief Centralized application logger (Singleton style).
 *
 * All parts of the application should use this class to emit log messages.
 * The UI (or any other listener) can connect to the logMessage signal
 * to receive and display logs.
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Returns the global logger instance.
     */
    static Logger& instance();

    /**
     * @brief Log a message with the given level.
     * @param level   Severity of the message
     * @param message Human-readable text
     * @param source  Optional source identifier (e.g. device id or module name)
     */
    void log(LogLevel level, const QString &message, const QString &source = QString());

    // Convenience helpers
    void debug(const QString &message, const QString &source = QString());
    void info(const QString &message, const QString &source = QString());
    void warning(const QString &message, const QString &source = QString());
    void error(const QString &message, const QString &source = QString());
    void critical(const QString &message, const QString &source = QString());

signals:
    /**
     * @brief Emitted whenever a new log entry is created.
     * @param level     Log severity
     * @param timestamp Formatted timestamp string
     * @param source    Origin of the message (can be empty)
     * @param message   The actual log text
     */
    void logMessage(edcc::LogLevel level,
                    const QString &timestamp,
                    const QString &source,
                    const QString &message);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger() override = default;

    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QMutex m_mutex;   // Protects concurrent access from multiple threads
};

} // namespace edcc
