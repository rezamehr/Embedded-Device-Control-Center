#include "DeviceManager.h"
#include <QUuid>

namespace edcc {

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
}

DeviceManager::~DeviceManager()
{
    removeAll();
}

QString DeviceManager::generateId() const
{
    return QString("dev_%1").arg(m_idCounter);
}

QString DeviceManager::addDevice(const DeviceInfo &info, ICommunication *communication)
{
    if (!communication) {
        return QString();
    }

    DeviceInfo finalInfo = info;
    if (finalInfo.id.isEmpty()) {
        finalInfo.id = generateId();
        const_cast<DeviceManager*>(this)->m_idCounter++;
    }

    if (finalInfo.name.isEmpty()) {
        finalInfo.name = communication->name();
    }

    auto *dev = new Device(finalInfo, communication, this);

    connect(dev, &Device::stateChanged,
            this, &DeviceManager::onDeviceStateChanged);
    connect(dev, &Device::dataReceived,
            this, &DeviceManager::onDeviceDataReceived);
    connect(dev, &Device::errorOccurred,
            this, &DeviceManager::onDeviceError);

    m_devices.insert(finalInfo.id, dev);
    emit deviceAdded(finalInfo.id);

    return finalInfo.id;
}

bool DeviceManager::removeDevice(const QString &id)
{
    if (!m_devices.contains(id)) {
        return false;
    }

    Device *dev = m_devices.take(id);
    dev->disconnectFromDevice();
    dev->deleteLater();

    emit deviceRemoved(id);
    return true;
}

Device* DeviceManager::device(const QString &id) const
{
    return m_devices.value(id, nullptr);
}

QList<Device*> DeviceManager::devices() const
{
    return m_devices.values();
}

int DeviceManager::deviceCount() const
{
    return m_devices.size();
}

void DeviceManager::disconnectAll()
{
    for (Device *dev : m_devices) {
        dev->disconnectFromDevice();
    }
}

void DeviceManager::removeAll()
{
    const QStringList ids = m_devices.keys();
    for (const QString &id : ids) {
        removeDevice(id);
    }
}

void DeviceManager::onDeviceStateChanged(ConnectionState state)
{
    Device *dev = qobject_cast<Device*>(sender());
    if (dev) {
        emit deviceStateChanged(dev->id(), state);
    }
}

void DeviceManager::onDeviceDataReceived(const QByteArray &data)
{
    Device *dev = qobject_cast<Device*>(sender());
    if (dev) {
        emit deviceDataReceived(dev->id(), data);
    }
}

void DeviceManager::onDeviceError(const QString &error)
{
    Device *dev = qobject_cast<Device*>(sender());
    if (dev) {
        emit deviceError(dev->id(), error);
    }
}

} // namespace edcc
