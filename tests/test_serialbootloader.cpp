/**
 * @file test_serialbootloader.cpp
 * @brief Unit tests for SerialBootloaderTarget (Device-based).
 *
 * Protocol: Bootloader Protocol v1.0
 *   Frame = START(0xAA) + LENGTH(1) + PAYLOAD + CHECKSUM(XOR of payload)
 */

#include <QtTest>

#include "core/Device.h"
#include "core/Types.h"
#include "firmware/SerialBootloaderTarget.h"
#include "firmware/protocol/BootloaderProtocol.h"
#include "MockCommunication.h"

using namespace edcc;
using namespace edcc::protocol;

// =============================================================================
// Helpers – build protocol responses for the mock
// =============================================================================

/**
 * @brief IDENTITY success response.
 *
 * Payload (12 bytes):
 *   ACK(1) + deviceId(2) + flashSize(4) + pageSize(4) + version(1)
 *
 * Frame:
 *   AA 0C FF 50 04 00 00 20 00 00 00 02 00 01 88
 */
static QByteArray makeIdentityResponse()
{
    QByteArray payload;

    // ACK
    payload.append(static_cast<char>(0xFF));

    // Device ID = 0x0450
    payload.append(static_cast<char>(0x50));
    payload.append(static_cast<char>(0x04));

    // Flash size = 0x00200000 (2 MB)
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(0x20));
    payload.append(static_cast<char>(0x00));

    // Page size = 0x00020000 (128 KB)
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(0x02));
    payload.append(static_cast<char>(0x00));

    // Version = 1
    payload.append(static_cast<char>(0x01));

    return buildFrame(payload); // uses 1-byte LENGTH
}

/**
 * @brief Simple ACK response: AA 01 FF FF
 */
static QByteArray makeAckResponse()
{
    QByteArray payload;
    payload.append(static_cast<char>(static_cast<quint8>(Command::Ack)));
    return buildFrame(payload);
}

/**
 * @brief Simple NACK response: AA 01 1F 1F
 */
static QByteArray makeNackResponse()
{
    QByteArray payload;
    payload.append(static_cast<char>(static_cast<quint8>(Command::Nack)));
    return buildFrame(payload);
}

/**
 * @brief READ success response = ACK + data
 */
static QByteArray makeReadResponse(const QByteArray &data)
{
    QByteArray payload;
    payload.append(static_cast<char>(static_cast<quint8>(Command::Ack)));
    payload.append(data);
    return buildFrame(payload);
}

// =============================================================================
// Test class
// =============================================================================

class TestSerialBootloader : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testBuildFrameIdentityExample();
    void testQueryIdentity();
    void testErase();
    void testEraseNack();
    void testWriteAndRead();
    void testJump();

private:
    MockCommunication *m_comm = nullptr;   ///< Owned by Device after construction
    Device *m_device = nullptr;
    SerialBootloaderTarget *m_target = nullptr;
};

// -----------------------------------------------------------------------------
// Setup / teardown
// -----------------------------------------------------------------------------

void TestSerialBootloader::init()
{
    m_comm = new MockCommunication();

    DeviceInfo info;
    info.id = QStringLiteral("mock-1");
    info.name = QStringLiteral("MockDevice");
    info.type = QStringLiteral("serial");
    info.portName = QStringLiteral("MOCK");
    info.baudRate = 115200;

    m_device = new Device(info, m_comm);

    QSignalSpy connectedSpy(m_device, &Device::stateChanged);
    QVERIFY(m_device->connectToDevice());

    // wait Connected
    QVERIFY(connectedSpy.wait(1000));
    QVERIFY(m_device->isConnected());

    m_target = new SerialBootloaderTarget(m_device);
}

void TestSerialBootloader::cleanup()
{
    delete m_target;
    m_target = nullptr;

    if (m_device) {
        m_device->disconnectFromDevice();
        delete m_device;   // also deletes m_comm
        m_device = nullptr;
        m_comm = nullptr;
    }
}

// -----------------------------------------------------------------------------
// Protocol sanity check (matches docs/bootloader_protocol.md)
// -----------------------------------------------------------------------------

void TestSerialBootloader::testBuildFrameIdentityExample()
{
    const QByteArray frame = makeIdentityResponse();

    // AA 0C FF 50 04 00 00 20 00 00 00 02 00 01 88
    QCOMPARE(frame.toHex(' '),
             QByteArrayLiteral("aa 0c ff 50 04 00 00 20 00 00 00 02 00 01 88"));

    QCOMPARE(static_cast<quint8>(frame.at(0)), quint8(0xAA));
    QCOMPARE(static_cast<quint8>(frame.at(1)), quint8(0x0C)); // LENGTH = 12
    QCOMPARE(static_cast<quint8>(frame.at(frame.size() - 1)), quint8(0x88));
}

// -----------------------------------------------------------------------------
// Target behavior tests
// -----------------------------------------------------------------------------

void TestSerialBootloader::testQueryIdentity()
{
    m_comm->setNextResponse(makeIdentityResponse());
   // QSignalSpy spy(m_device, &Device::dataReceived);
   // m_comm->setNextResponse(makeIdentityResponse());
   // m_target->queryIdentity();
   // qDebug() << "device dataReceived count:" << spy.count();

    QVERIFY(m_target->queryIdentity());
    QVERIFY(m_target->isReady());

    const MemoryTargetInfo info = m_target->info();
    QCOMPARE(info.size, 0x200000u);     // 2 MB
    QCOMPARE(info.pageSize, 0x20000u);  // 128 KB
}

void TestSerialBootloader::testErase()
{
    m_comm->setNextResponse(makeIdentityResponse());
    QVERIFY(m_target->queryIdentity());

    m_comm->setNextResponse(makeAckResponse());
    QVERIFY(m_target->erase(0x08004000, 0x10000));
}

void TestSerialBootloader::testEraseNack()
{
    m_comm->setNextResponse(makeIdentityResponse());
    QVERIFY(m_target->queryIdentity());

    m_comm->setNextResponse(makeNackResponse());
    QVERIFY(!m_target->erase(0x08004000, 0x10000));
}

void TestSerialBootloader::testWriteAndRead()
{
    m_comm->setNextResponse(makeIdentityResponse());
    QVERIFY(m_target->queryIdentity());

    const QByteArray data = QByteArray::fromHex("1122334455667788");

    // WRITE → ACK
    m_comm->setNextResponse(makeAckResponse());
    QVERIFY(m_target->write(0x08004000, data));

    // READ → ACK + data
    m_comm->setNextResponse(makeReadResponse(data));
    const QByteArray readBack =
        m_target->read(0x08004000, static_cast<quint32>(data.size()));

    QCOMPARE(readBack, data);
}

void TestSerialBootloader::testJump()
{
    m_comm->setNextResponse(makeIdentityResponse());
    QVERIFY(m_target->queryIdentity());
    QVERIFY(m_target->isReady());

    m_comm->setNextResponse(makeAckResponse());
    QVERIFY(m_target->jumpTo(0x08004000));
    QVERIFY(!m_target->isReady());
}

// =============================================================================
// Qt Test entry point
// =============================================================================

QTEST_MAIN(TestSerialBootloader)
#include "test_serialbootloader.moc"
