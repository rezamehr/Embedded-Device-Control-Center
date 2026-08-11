#include <QtTest>
#include <QDataStream>
#include <QIODevice>

#include "firmware/SerialBootloaderTarget.h"
#include "firmware/protocol/BootloaderProtocol.h"
#include "MockCommunication.h"

using namespace edcc;
using namespace edcc::protocol;

// ---------------------------------------------------------------------------
// Test helpers
// ---------------------------------------------------------------------------

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

    return buildFrame(payload);
}

static QByteArray makeAckResponse()
{
    QByteArray payload;
    payload.append(static_cast<char>(0xFF));
    return buildFrame(payload);
}

// ---------------------------------------------------------------------------
// Test class
// ---------------------------------------------------------------------------

class TestSerialBootloader : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testQueryIdentity();
    void testErase();
    void testWriteAndRead();
    void testJump();

private:
    MockCommunication *m_comm = nullptr;
    SerialBootloaderTarget *m_target = nullptr;
};

void TestSerialBootloader::init()
{
    m_comm = new MockCommunication();
    m_comm->open();
    m_target = new SerialBootloaderTarget(m_comm);
}

void TestSerialBootloader::cleanup()
{
    delete m_target;
    delete m_comm;
    m_target = nullptr;
    m_comm = nullptr;
}

void TestSerialBootloader::testQueryIdentity()
{
    m_comm->setNextResponse(makeIdentityResponse());

    QVERIFY(m_target->queryIdentity());
    QVERIFY(m_target->isReady());

    const auto info = m_target->info();
    QCOMPARE(info.size, 0x200000u);
    QCOMPARE(info.pageSize, 0x20000u);
}

void TestSerialBootloader::testErase()
{
    m_comm->setNextResponse(makeIdentityResponse());
    QVERIFY(m_target->queryIdentity());

    m_comm->setNextResponse(makeAckResponse());
    QVERIFY(m_target->erase(0x08000000, 0x10000));
}

void TestSerialBootloader::testWriteAndRead()
{
    m_comm->setNextResponse(makeIdentityResponse());
    QVERIFY(m_target->queryIdentity());

    const QByteArray data = QByteArray::fromHex("1122334455667788");

    m_comm->setNextResponse(makeAckResponse());
    QVERIFY(m_target->write(0x08004000, data));

    QByteArray readPayload;
    readPayload.append(static_cast<char>(0xFF)); // ACK
    readPayload.append(data);

    m_comm->setNextResponse(buildFrame(readPayload));

    const QByteArray readBack =
        m_target->read(0x08004000, static_cast<quint32>(data.size()));
    QCOMPARE(readBack, data);
}

void TestSerialBootloader::testJump()
{
    m_comm->setNextResponse(makeIdentityResponse());
    QVERIFY(m_target->queryIdentity());

    m_comm->setNextResponse(makeAckResponse());
    QVERIFY(m_target->jumpTo(0x08004000));
    QVERIFY(!m_target->isReady());
}

QTEST_MAIN(TestSerialBootloader)
#include "test_serialbootloader.moc"
