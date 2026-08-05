#pragma once

#include "IDevice.h"
#include "ICommunication.h"
#include <QSharedPointer>

namespace edcc {

class Device : public IDevice
{
    Q_OBJECT

public:
    explicit Device(const DeviceInfo &info,
                    ICommunication *communication,
                    QObject *parent = nullptr);
    ~Device() override;

    QString id() const override;
    QString name() const override;
    QString description() const override;

    bool connectToDevice() override;
    void disconnectFromDevice() override;
    bool isConnected() const override;
    ConnectionState state() const override;

    qint64 send(const QByteArray &data);

private slots:
    void onCommStateChanged(ConnectionState state);
    void onCommDataReceived(const QByteArray &data);
    void onCommError(const QString &error);

private:
    DeviceInfo m_info;
    ICommunication *m_comm;      // owned by this Device
    ConnectionState m_state = ConnectionState::Disconnected;
};

} // namespace edcc
