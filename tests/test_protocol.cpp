#include <QtTest>
#include "communication/SimplePacketParser.h"

using namespace edcc;

/**
 * @brief Unit tests for SimplePacketParser
 *
 * Frame format:
 *   START (0xAA) + LENGTH (1 byte) + PAYLOAD + CHECKSUM (XOR of length+payload)
 */
class TestProtocol : public QObject
{
    Q_OBJECT

private slots:
    void init();                 // runs before each test
    void cleanup();              // runs after each test

    void testValidPacket();
    void testValidPacketInChunks();
    void testWrongChecksum();
    void testEmptyPayload();
    void testGarbageThenValidPacket();
    void testResetClearsState();
    void testIdentityFrameFromSpec();

private:
    SimplePacketParser *m_parser = nullptr;
    QByteArray m_lastPacket;
    QString m_lastError;
    int m_packetCount = 0;
    int m_errorCount = 0;
/** Build a valid frame with payload-only XOR checksum. */
    static QByteArray buildFrame(const QByteArray &payload);
};

QByteArray TestProtocol::buildFrame(const QByteArray &payload)
{
    QByteArray frame;
    frame.append(static_cast<char>(SimplePacketParser::START_BYTE));
    frame.append(static_cast<char>(payload.size() & 0xFF)); // LENGTH = 1 byte

    quint8 checksum = 0;
    for (unsigned char b : payload) {
        frame.append(static_cast<char>(b));
        checksum ^= b;   // XOR payload only (NOT length)
    }
    frame.append(static_cast<char>(checksum));
    return frame;
}

void TestProtocol::init()
{
    m_parser = new SimplePacketParser();
    m_lastPacket.clear();
    m_lastError.clear();
    m_packetCount = 0;
    m_errorCount = 0;

    QObject::connect(m_parser, &SimplePacketParser::packetReady,
                     this, [this](const QByteArray &packet) {
                         m_lastPacket = packet;
                         ++m_packetCount;
                     });

    QObject::connect(m_parser, &SimplePacketParser::parseError,
                     this, [this](const QString &reason) {
                         m_lastError = reason;
                         ++m_errorCount;
                     });
}

void TestProtocol::cleanup()
{
    delete m_parser;
    m_parser = nullptr;
}

void TestProtocol::testValidPacket()
{
    const QByteArray payload = QByteArray::fromHex("010203");
    m_parser->feed(buildFrame(payload));

    QCOMPARE(m_packetCount, 1);//QCOMPARE(actual, expected);
    QCOMPARE(m_errorCount, 0);//none Error
    QCOMPARE(m_lastPacket, payload);//Right Data
}

void TestProtocol::testValidPacketInChunks()
{
    const QByteArray payload = QByteArray::fromHex("0A0B0C");
    const QByteArray frame = buildFrame(payload);

    // Feed one byte at a time
    for (char b : frame) {
        m_parser->feed(QByteArray(1, b));
    }

    QCOMPARE(m_packetCount, 1);
    QCOMPARE(m_lastPacket, payload);
}

void TestProtocol::testWrongChecksum()
{
    QByteArray frame = buildFrame(QByteArray::fromHex("010203"));
    // Corrupt last byte (checksum)
    frame[frame.size() - 1] = char(0x00);

    m_parser->feed(frame);

    QCOMPARE(m_packetCount, 0);//receive none packet
    QCOMPARE(m_errorCount, 1);//one error
    QVERIFY(!m_lastError.isEmpty());//QVERIFY(condition);//Right condition
}

void TestProtocol::testEmptyPayload()
{
    // LENGTH = 0, checksum = 0
    QByteArray frame;
    frame.append(static_cast<char>(SimplePacketParser::START_BYTE));
    frame.append(char(0x00)); // length
    frame.append(char(0x00)); // checksum

    m_parser->feed(frame);

    QCOMPARE(m_packetCount, 1);
    QCOMPARE(m_lastPacket.size(), 0);
    QCOMPARE(m_errorCount, 0);
}

void TestProtocol::testGarbageThenValidPacket()
{
    const QByteArray payload = QByteArray::fromHex("1122");
    QByteArray stream;
    stream.append("xyz");                 // garbage
    stream.append(buildFrame(payload));   // valid frame

    m_parser->feed(stream);

    QCOMPARE(m_packetCount, 1);//receive one packet
    QCOMPARE(m_lastPacket, payload);//Right data
}

void TestProtocol::testResetClearsState()
{
    // Start a frame but do not finish it
    m_parser->feed(QByteArray::fromHex("AA03"));
    m_parser->reset();

    // Now send a full valid frame
    const QByteArray payload = QByteArray::fromHex("01");
    m_parser->feed(buildFrame(payload));

    QCOMPARE(m_packetCount, 1);
    QCOMPARE(m_lastPacket, payload);
}
/**
 * @brief Golden frame from docs/bootloader_protocol.md (IDENTITY response)
 *
 * AA 0C FF 50 04 00 00 20 00 00 00 02 00 01 88
 */
void TestProtocol::testIdentityFrameFromSpec()
{
    const QByteArray frame =
        QByteArray::fromHex("AA0CFF500400002000000002000188");

    m_parser->feed(frame);

    QCOMPARE(m_packetCount, 1);
    QCOMPARE(m_errorCount, 0);

    const QByteArray expectedPayload =
        QByteArray::fromHex("FF5004000020000000020001");
    QCOMPARE(m_lastPacket, expectedPayload);
}
//QVERIFY2(condition, "message");
//QCOMPARE(actual, expected, 0.001); Comparison with approximate value (for float/double)
//QFAIL("This test should not reach here"); Auto FAIL
//QCOMPARE(qstr, "expected");comparison string
QTEST_MAIN(TestProtocol)
#include "test_protocol.moc"
