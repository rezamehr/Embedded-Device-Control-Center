#include "DeviceManager.h"
#include <QUuid>

#include "utils/JsonConfig.h"
#include "communication/SerialCommunication.h"
#include "communication/TcpCommunication.h"
#include "logging/Logger.h"

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

    connect(dev, &Device::packetReceived, this, [this, id = finalInfo.id](const QByteArray &packet) {
        emit devicePacketReceived(id, packet);
    });

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

bool DeviceManager::saveToConfig() const
{
    QVector<DeviceInfo> list;
    for (Device *dev : m_devices) {
        list.append(dev->info());   // full configuration
    }
    return JsonConfig::saveDevices(list);
}

int DeviceManager::loadFromConfig()
{
    const QVector<DeviceInfo> list = JsonConfig::loadDevices();
    int count = 0;

    Logger::instance().info(QString("Loading %1 device(s) from config...").arg(list.size()));

    for (const DeviceInfo &info : list) {
        ICommunication *comm = nullptr;

        if (info.type == "serial" || !info.portName.isEmpty()) {
            if (info.portName.isEmpty()) {
                Logger::instance().error("Skip device: empty portName", info.id);
                continue;
            }

            Logger::instance().debug(
                QString("Create Serial: port=%1 baud=%2").arg(info.portName).arg(info.baudRate),
                info.id);

            comm = new SerialCommunication(info.portName,
                                           info.baudRate > 0 ? info.baudRate : 115200);
        }
        else if (info.type == "tcp" || !info.host.isEmpty()) {
            if (info.host.isEmpty() || info.port == 0) {
                Logger::instance().error("Skip device: invalid host/port", info.id);
                continue;
            }

            Logger::instance().debug(
                QString("Create TCP: %1:%2").arg(info.host).arg(info.port),
                info.id);

            comm = new TcpCommunication(info.host, info.port);
        }
        else {
            Logger::instance().error("Skip device: unknown type", info.id);
            continue;
        }

        // Keep the original saved id
        DeviceInfo copy = info;
        const QString id = addDevice(copy, comm);

        if (id.isEmpty()) {
            Logger::instance().error("Failed to add loaded device", info.id);
            delete comm;
            continue;
        }

        Logger::instance().info("Loaded device OK", id);
        ++count;
    }

    return count;
}

void DeviceManager::onDeviceError(const QString &error)
{
    Device *dev = qobject_cast<Device*>(sender());
    if (dev) {
        emit deviceError(dev->id(), error);
    }
}

} // namespace edcc
