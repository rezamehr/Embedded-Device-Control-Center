#include "SerialBootloaderTarget.h"
#include "protocol/BootloaderProtocol.h"
#include "logging/Logger.h"

#include <QDataStream>
#include <QIODevice>

namespace edcc {

using namespace protocol;

SerialBootloaderTarget::SerialBootloaderTarget(ICommunication *communication,
                                               QObject *parent)
    : IMemoryTarget(parent)
    , m_comm(communication)
{
    Q_ASSERT(m_comm != nullptr);

    connect(m_comm, &ICommunication::dataReceived,
            this, &SerialBootloaderTarget::onDataReceived);

    // Sensible defaults until queryIdentity() updates them
    m_info.name = QStringLiteral("Serial Bootloader Target");
    m_info.startAddress = 0x08000000;
    m_info.size = 1024 * 1024;
    m_info.pageSize = 128 * 1024;
    m_info.writeAlignment = 32;
    m_info.maxWriteChunk = 256;
    m_info.supportsDualBank = true;
}

SerialBootloaderTarget::~SerialBootloaderTarget() = default;

MemoryTargetInfo SerialBootloaderTarget::info() const
{
    return m_info;
}

bool SerialBootloaderTarget::isReady() const
{
    return m_ready && m_comm && m_comm->isOpen();
}

bool SerialBootloaderTarget::queryIdentity()
{
    QByteArray response;
    const QByteArray frame = buildCommand(Command::Identity);

    if (!sendCommandAndWait(frame, response, 1500)) {
        m_ready = false;
        return false;
    }

    // ACK(1) + deviceId(2) + flashSize(4) + pageSize(4) + version(1) = 12
    if (response.size() < 12 ||
        static_cast<quint8>(response.at(0)) != static_cast<quint8>(Command::Ack)) {
        emit errorOccurred(tr("Invalid identity response"));
        m_ready = false;
        return false;
    }

    QDataStream stream(response.mid(1));
    stream.setByteOrder(QDataStream::LittleEndian);

    quint16 deviceId = 0;
    quint32 flashSize = 0;
    quint32 pageSize = 0;
    quint8 version = 0;

    stream >> deviceId >> flashSize >> pageSize >> version;

    if (stream.status() != QDataStream::Ok) {
        emit errorOccurred(tr("Failed to parse identity response"));
        m_ready = false;
        return false;
    }

    m_info.name = QStringLiteral("STM32 Bootloader (ID: 0x%1)")
                      .arg(deviceId, 4, 16, QChar('0'));
    m_info.size = flashSize;
    m_info.pageSize = pageSize;
    m_ready = true;

    Logger::instance().info(
        QStringLiteral("SerialBootloaderTarget"),
        tr("Identity OK - Flash: %1 KB, Page: %2 bytes, Ver: %3")
            .arg(flashSize / 1024)
            .arg(pageSize)
            .arg(version));

    return true;
}

bool SerialBootloaderTarget::erase(quint32 address, quint32 size)
{
    if (!isReady()) {
        return false;
    }

    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << address << size;

    const QByteArray frame = buildCommand(Command::Erase, payload);

    QByteArray response;
    if (!sendCommandAndWait(frame, response, 5000)) {
        return false;
    }

    return !response.isEmpty()
           && static_cast<quint8>(response.at(0)) == static_cast<quint8>(Command::Ack);
}

bool SerialBootloaderTarget::massErase()
{
    return erase(m_info.startAddress, m_info.size);
}

bool SerialBootloaderTarget::write(quint32 address, const QByteArray &data)
{
    if (!isReady() || data.isEmpty()) {
        return false;
    }

    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << address;
    payload.append(data);

    const QByteArray frame = buildCommand(Command::Write, payload);

    QByteArray response;
    if (!sendCommandAndWait(frame, response, 3000)) {
        return false;
    }

    return !response.isEmpty()
           && static_cast<quint8>(response.at(0)) == static_cast<quint8>(Command::Ack);
}

QByteArray SerialBootloaderTarget::read(quint32 address, quint32 size)
{
    if (!isReady() || size == 0) {
        return {};
    }

    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << address << static_cast<quint16>(size);

    const QByteArray frame = buildCommand(Command::Read, payload);

    QByteArray response;
    if (!sendCommandAndWait(frame, response, 3000)) {
        return {};
    }

    if (response.isEmpty()
        || static_cast<quint8>(response.at(0)) != static_cast<quint8>(Command::Ack)) {
        return {};
    }

    return response.mid(1);
}

bool SerialBootloaderTarget::verify(quint32 address, const QByteArray &expected)
{
    const QByteArray actual = read(address, static_cast<quint32>(expected.size()));
    return actual == expected;
}

bool SerialBootloaderTarget::jumpTo(quint32 address)
{
    if (!isReady()) {
        return false;
    }

    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << address;

    const QByteArray frame = buildCommand(Command::Jump, payload);

    // Device may reset immediately after JUMP; ACK is optional
    QByteArray response;
    sendCommandAndWait(frame, response, 1000);

    m_ready = false;
    return true;
}

void SerialBootloaderTarget::onDataReceived(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_receiveBuffer.append(data);

    while (m_receiveBuffer.size() >= 4) {
        if (static_cast<quint8>(m_receiveBuffer.at(0)) != FRAME_START) {
            m_receiveBuffer.remove(0, 1);
            continue;
        }

        const quint16 len = static_cast<quint8>(m_receiveBuffer.at(1))
                            | (static_cast<quint8>(m_receiveBuffer.at(2)) << 8);

        const int totalSize = 3 + len + 1;
        if (m_receiveBuffer.size() < totalSize) {
            break;
        }

        const QByteArray payload = m_receiveBuffer.mid(3, len);
        const quint8 receivedChecksum =
            static_cast<quint8>(m_receiveBuffer.at(3 + len));
        const quint8 calculated = calculateChecksum(payload);

        m_receiveBuffer.remove(0, totalSize);

        if (receivedChecksum == calculated) {
            m_responsePayload = payload;
            m_responseReady = true;
            m_condition.wakeAll();
        } else {
            Logger::instance().warning(
                QStringLiteral("SerialBootloaderTarget"),
                tr("Checksum mismatch"));
        }
    }
}

bool SerialBootloaderTarget::sendCommandAndWait(const QByteArray &frame,
                                                QByteArray &responsePayload,
                                                int timeoutMs)
{
    {
        QMutexLocker locker(&m_mutex);
        m_responsePayload.clear();
        m_responseReady = false;
    }

    if (!m_comm || m_comm->send(frame) <= 0) {
        emit errorOccurred(tr("Failed to send command"));
        return false;
    }

    QMutexLocker locker(&m_mutex);

    if (!m_responseReady) {
        if (!m_condition.wait(&m_mutex, timeoutMs)) {
            emit errorOccurred(tr("Timeout waiting for response"));
            return false;
        }
    }

    responsePayload = m_responsePayload;
    return true;
}

} // namespace edcc
