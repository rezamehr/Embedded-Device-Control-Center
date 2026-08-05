#include "Device.h"

namespace edcc {

Device::Device(const DeviceInfo &info,
               ICommunication *communication,
               QObject *parent)
    : IDevice(parent)
    , m_info(info)
    , m_comm(communication)
{
    if (m_comm) {
        m_comm->setParent(this);   // ownership

        connect(m_comm, &ICommunication::stateChanged,
                this, &Device::onCommStateChanged);
        connect(m_comm, &ICommunication::dataReceived,
                this, &Device::onCommDataReceived);
        connect(m_comm, &ICommunication::errorOccurred,
                this, &Device::onCommError);
    }
}

Device::~Device()
{
    disconnectFromDevice();
}

QString Device::id() const { return m_info.id; }
QString Device::name() const { return m_info.name; }
QString Device::description() const { return m_info.description; }

bool Device::connectToDevice()
{
    if (!m_comm) return false;
    return m_comm->open();
}

void Device::disconnectFromDevice()
{
    if (m_comm) {
        m_comm->close();
    }
}

bool Device::isConnected() const
{
    return m_state == ConnectionState::Connected;
}

ConnectionState Device::state() const
{
    return m_state;
}

qint64 Device::send(const QByteArray &data)
{
    if (!m_comm || !isConnected()) return -1;
    return m_comm->send(data);
}

void Device::onCommStateChanged(ConnectionState state)
{
    m_state = state;
    emit stateChanged(state);
}

void Device::onCommDataReceived(const QByteArray &data)
{
    emit dataReceived(data);
}

void Device::onCommError(const QString &error)
{
    emit errorOccurred(error);
}

} // namespace edcc
