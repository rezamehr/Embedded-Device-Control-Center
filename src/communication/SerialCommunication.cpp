
#include "SerialCommunication.h"
#include <QDebug>

namespace edcc {

SerialCommunication::SerialCommunication(const QString &portName,
                                         qint32 baudRate,
                                         QObject *parent)
    : ICommunication(parent)
    , m_serial(new QSerialPort(this))
    , m_portName(portName)
    , m_baudRate(baudRate)
{
    connect(m_serial, &QSerialPort::readyRead,
            this, &SerialCommunication::onReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred,
            this, &SerialCommunication::onErrorOccurred);
}

SerialCommunication::~SerialCommunication()
{
    close();
}

bool SerialCommunication::open()
{
    if (m_serial->isOpen()) {
        return true;
    }

    m_serial->setPortName(m_portName);
    m_serial->setBaudRate(m_baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        emit stateChanged(ConnectionState::Connected);
        return true;
    }

    emit stateChanged(ConnectionState::Error);
    emit errorOccurred(m_serial->errorString());
    return false;
}

void SerialCommunication::close()
{
    if (m_serial->isOpen()) {
        m_serial->close();
        emit stateChanged(ConnectionState::Disconnected);
    }
}

bool SerialCommunication::isOpen() const
{
    return m_serial->isOpen();
}

qint64 SerialCommunication::send(const QByteArray &data)
{
    if (!m_serial->isOpen()) {
        return -1;
    }
    return m_serial->write(data);
}

QString SerialCommunication::name() const
{
    return QString("Serial(%1)").arg(m_portName);
}

void SerialCommunication::setBaudRate(qint32 baudRate)
{
    m_baudRate = baudRate;
    if (m_serial->isOpen()) {
        m_serial->setBaudRate(baudRate);
    }
}

qint32 SerialCommunication::baudRate() const
{
    return m_baudRate;
}

void SerialCommunication::onReadyRead()
{
    const QByteArray data = m_serial->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void SerialCommunication::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        close();
        emit errorOccurred(m_serial->errorString());
        emit stateChanged(ConnectionState::Error);
    }
}

} // namespace edcc
