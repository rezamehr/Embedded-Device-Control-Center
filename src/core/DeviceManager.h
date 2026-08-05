#pragma once

#include "Device.h"
#include <QObject>
#include <QMap>
#include <QList>

namespace edcc {

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager() override;

    // Device management
    QString addDevice(const DeviceInfo &info, ICommunication *communication);
    bool removeDevice(const QString &id);
    Device* device(const QString &id) const;
    QList<Device*> devices() const;
    int deviceCount() const;

    // Bulk operations
    void disconnectAll();
    void removeAll();

signals:
    void deviceAdded(const QString &id);
    void deviceRemoved(const QString &id);
    void deviceStateChanged(const QString &id, edcc::ConnectionState state);
    void deviceDataReceived(const QString &id, const QByteArray &data);
    void deviceError(const QString &id, const QString &error);

private slots:
    void onDeviceStateChanged(edcc::ConnectionState state);
    void onDeviceDataReceived(const QByteArray &data);
    void onDeviceError(const QString &error);

private:
    QString generateId() const;

    QMap<QString, Device*> m_devices;
    int m_idCounter = 1;
};

} // namespace edcc
