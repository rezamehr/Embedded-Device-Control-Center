#include "SerialBootloaderTarget.h"
#include "protocol/BootloaderProtocol.h"
#include "logging/Logger.h"

#include <QDataStream>
#include <QIODevice>
#include <QEventLoop>
#include <QTimer>

namespace edcc {

using namespace protocol;

SerialBootloaderTarget::SerialBootloaderTarget(Device *device, QObject *parent)
    : IMemoryTarget(parent)
    , m_device(device)
{
    Q_ASSERT(m_device != nullptr);

    connect(m_device, &Device::dataReceived,
            this, &SerialBootloaderTarget::onDeviceDataReceived);

    m_info.name = QStringLiteral("Device Bootloader Target");
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
    return m_ready && m_device && m_device->isConnected();
}

bool SerialBootloaderTarget::queryIdentity()
{
    QByteArray response;
    const QByteArray frame = buildCommand(Command::Identity);

    if (!sendCommandAndWait(frame, response, 3000)) {
        m_ready = false;
        Logger::instance().info(
            QStringLiteral("SerialBootloader"),
            tr("m_ready: %1")
                .arg(false));
        return false;
    }

    // ACK(1) + deviceId(2) + flashSize(4) + pageSize(4) + version(1) = 12
    //AA 0C FF 50 04 00 00 20 00 00 00 02 00 01 88
    if (response.size() < 12 ||
        static_cast<quint8>(response.at(0)) != static_cast<quint8>(Command::Ack)) {
        emit errorOccurred(tr("Invalid identity response"));
        m_ready = false;
        Logger::instance().info(
            QStringLiteral("SerialBootloader"),
            tr("response.size: %1")
                .arg(false));
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
        Logger::instance().info(
            QStringLiteral("SerialBootloader"),
            tr("QDataStream: %1")
                .arg(false));
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
    //AA 09 03 00 40 00 08 00 00 01 00 4A
    //AA 01 FF FF
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

void SerialBootloaderTarget::onDeviceDataReceived(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);

    Logger::instance().debug(
        QStringLiteral("SerialBootloaderTarget"),
        tr("RX raw (%1 bytes): %2")
            .arg(data.size())
            .arg(QString(data.toHex(' '))));

    m_receiveBuffer.append(data);

    Logger::instance().debug(
        QStringLiteral("SerialBootloaderTarget"),
        tr("Buffer now (%1 bytes): %2")
            .arg(m_receiveBuffer.size())
            .arg(QString(m_receiveBuffer.toHex(' '))));

    while (m_receiveBuffer.size() >= 3) {
        const quint8 start = static_cast<quint8>(m_receiveBuffer.at(0));

        if (start != FRAME_START) {
            Logger::instance().debug(
                QStringLiteral("SerialBootloaderTarget"),
                tr("Skip byte (not START): 0x%1")
                    .arg(start, 2, 16, QChar('0')));
            m_receiveBuffer.remove(0, 1);
            continue;
        }

        const quint8 len = static_cast<quint8>(m_receiveBuffer.at(1));
        const int totalSize = 2 + len + 1; // START + LEN + PAYLOAD + CHECKSUM

        Logger::instance().debug(
            QStringLiteral("SerialBootloaderTarget"),
            tr("Frame header: START=0xAA LEN=%1 totalSize=%2 buffer=%3")
                .arg(len)
                .arg(totalSize)
                .arg(m_receiveBuffer.size()));

        if (m_receiveBuffer.size() < totalSize) {
            Logger::instance().debug(
                QStringLiteral("SerialBootloaderTarget"),
                tr("Waiting for more bytes... need %1 have %2")
                    .arg(totalSize)
                    .arg(m_receiveBuffer.size()));
            break;
        }

        const QByteArray payload = m_receiveBuffer.mid(2, len);
        const quint8 receivedChecksum =
            static_cast<quint8>(m_receiveBuffer.at(2 + len));
        const quint8 calculated = calculateChecksum(payload);

        Logger::instance().debug(
            QStringLiteral("SerialBootloaderTarget"),
            tr("Payload (%1): %2")
                .arg(payload.size())
                .arg(QString(payload.toHex(' '))));

        Logger::instance().debug(
            QStringLiteral("SerialBootloaderTarget"),
            tr("Checksum received=0x%1 calculated=0x%2 %3")
                .arg(receivedChecksum, 2, 16, QChar('0'))
                .arg(calculated, 2, 16, QChar('0'))
                .arg(receivedChecksum == calculated ? "OK" : "FAIL"));

        // بایت‌به‌بایت XOR برای دیباگ
        {
            QString xorTrace;
            quint8 running = 0;
            for (int i = 0; i < payload.size(); ++i) {
                const quint8 b = static_cast<quint8>(payload.at(i));
                running ^= b;
                xorTrace += QString("0x%1->0x%2 ")
                                .arg(b, 2, 16, QChar('0'))
                                .arg(running, 2, 16, QChar('0'));
            }
            Logger::instance().debug(
                QStringLiteral("SerialBootloaderTarget"),
                tr("XOR trace: %1").arg(xorTrace.trimmed()));
        }

        m_receiveBuffer.remove(0, totalSize);

        if (receivedChecksum == calculated) {
            m_responsePayload = payload;
            m_responseReady = true;
            m_condition.wakeAll();

            Logger::instance().info(
                QStringLiteral("SerialBootloaderTarget"),
                tr("Valid frame accepted (%1 byte payload)").arg(payload.size()));
        } else {
            Logger::instance().warning(
                QStringLiteral("SerialBootloaderTarget"),
                tr("Checksum mismatch – frame dropped"));
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
        m_receiveBuffer.clear();
    }

    if (!m_device || !m_device->isConnected()) {
        emit errorOccurred(tr("Device is not connected"));
        return false;
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    const auto connResponse = connect(this, &SerialBootloaderTarget::responseReceived,
                                      &loop, &QEventLoop::quit);
    const auto connTimeout = connect(&timer, &QTimer::timeout,
                                     &loop, &QEventLoop::quit);

    m_device->send(frame);
    timer.start(timeoutMs);

    // این Event Loop را زنده نگه می‌دارد تا dataReceived پردازش شود
    loop.exec();

    disconnect(connResponse);
    disconnect(connTimeout);

    QMutexLocker locker(&m_mutex);
    if (!m_responseReady) {
        emit errorOccurred(tr("Timeout waiting for response"));
        return false;
    }

    responsePayload = m_responsePayload;
    return true;
}

} // namespace edcc
