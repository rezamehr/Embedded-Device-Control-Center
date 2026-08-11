#include <QtTest>
#include "firmware/FirmwareUpdater.h"
#include "firmware/DummyMemoryTarget.h"

using namespace edcc;

class TestFirmware : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testSuccessfulUpdate();
    void testEmptyFirmware();
    void testAbort();
    void testVerifyFailure();
    void testNotReady();

private:
    DummyMemoryTarget *m_target = nullptr;
    FirmwareUpdater *m_updater = nullptr;
};

void TestFirmware::init()
{
    m_target = new DummyMemoryTarget();
    m_updater = new FirmwareUpdater(m_target);
}

void TestFirmware::cleanup()
{
    delete m_updater;
    delete m_target;
    m_updater = nullptr;
    m_target = nullptr;
}

void TestFirmware::testSuccessfulUpdate()
{
    // Create a simple fake firmware
    QByteArray firmware;
    for (int i = 0; i < 1024; ++i) {
        firmware.append(static_cast<char>(i & 0xFF));
    }

    const quint32 startAddress = 0x08004000;

    QSignalSpy finishedSpy(m_updater, &FirmwareUpdater::finished);
    QSignalSpy progressSpy(m_updater, &FirmwareUpdater::progressChanged);

    const bool started = m_updater->startUpdate(firmware, startAddress);
    QVERIFY(started);

    // Wait for finished signal
   // QVERIFY(finishedSpy.wait(1000));
    // we nned to wait because signal finished before
    QCOMPARE(finishedSpy.count(), 1);

    const QList<QVariant> arguments = finishedSpy.takeFirst();
    const bool success = arguments.at(0).toBool();
    QVERIFY(success);

    // Verify data was written correctly
    const QByteArray readBack = m_target->read(startAddress, firmware.size());
    QCOMPARE(readBack, firmware);
}

void TestFirmware::testEmptyFirmware()
{
    QSignalSpy finishedSpy(m_updater, &FirmwareUpdater::finished);
    QSignalSpy errorSpy(m_updater, &FirmwareUpdater::errorOccurred);

    const bool started = m_updater->startUpdate(QByteArray(), 0x08004000);
    QVERIFY(!started);

    QVERIFY(errorSpy.count() > 0);
}

void TestFirmware::testAbort()
{
    // Large firmware to give us time to abort
    QByteArray firmware(64 * 1024, 0xAA);
    const quint32 startAddress = 0x08004000;

    QSignalSpy finishedSpy(m_updater, &FirmwareUpdater::finished);

    // Start update in a way that we can abort quickly
    // Note: current implementation is synchronous, so abort is limited.
    // This test mainly checks that abort() doesn't crash.
    m_updater->startUpdate(firmware, startAddress);
    m_updater->abort();

    // For now just ensure no crash
    QVERIFY(true);
}

void TestFirmware::testVerifyFailure()
{
    // We can force verify failure by manually corrupting memory after write,
    // but since write+verify happens inside startUpdate, we test indirectly.

    QByteArray firmware(512, 0x55);
    const quint32 startAddress = 0x08004000;

    // First do a normal update
    QVERIFY(m_updater->startUpdate(firmware, startAddress));

    // Manually corrupt one byte
    QByteArray corrupted = m_target->read(startAddress, 1);
    corrupted[0] = static_cast<char>(corrupted[0] ^ 0xFF);
    m_target->write(startAddress, corrupted);

    // Now verify should fail
    const bool verified = m_target->verify(startAddress, firmware);
    QVERIFY(!verified);
}

void TestFirmware::testNotReady()
{
    m_target->setReady(false);

    QSignalSpy errorSpy(m_updater, &FirmwareUpdater::errorOccurred);

    const bool started = m_updater->startUpdate(QByteArray(100, 0x11), 0x08004000);
    QVERIFY(!started);
    QVERIFY(errorSpy.count() > 0);
}

QTEST_MAIN(TestFirmware)
#include "test_firmware.moc"
