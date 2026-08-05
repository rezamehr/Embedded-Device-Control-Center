#pragma once

#include <QString>
#include <QByteArray>

namespace edcc {

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Error
};

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

struct DeviceInfo {
    QString id;
    QString name;
    QString description;
    QString portName;      // for serial
    QString host;          // for TCP
    quint16 port = 0;      // for TCP
};

} // namespace edcc
