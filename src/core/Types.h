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
    qint32  baudRate = 115200;   // for serial
    QString type;          // "serial" or "tcp"
};

} // namespace edcc
