#pragma once

#include <QObject>
#include <QString>
#include "Types.h"

namespace edcc {

class IDevice : public QObject
{
    Q_OBJECT

public:
    explicit IDevice(QObject *parent = nullptr) : QObject(parent) {}
    ~IDevice() override = default;

    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;

    virtual bool connectToDevice() = 0;
    virtual void disconnectFromDevice() = 0;
    virtual bool isConnected() const = 0;

    virtual edcc::ConnectionState state() const = 0;

signals:
    void stateChanged(edcc::ConnectionState state);
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &errorString);
};

} // namespace edcc
