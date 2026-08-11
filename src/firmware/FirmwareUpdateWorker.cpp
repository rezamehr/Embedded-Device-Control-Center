#include "FirmwareUpdateWorker.h"
#include "logging/Logger.h"

namespace edcc {

FirmwareUpdateWorker::FirmwareUpdateWorker(IMemoryTarget *target, QObject *parent)
    : QObject(parent)
    , m_target(target)
{
    Q_ASSERT(m_target != nullptr);
}

void FirmwareUpdateWorker::doUpdate(const QByteArray &firmware, quint32 startAddress, quint32 chunkSize)
{
    m_abortRequested = false;

    if (!m_target || !m_target->isReady()) {
        emit errorOccurred(tr("Memory target is not ready"));
        emit finished(false, tr("Target not ready"));
        return;
    }

    if (firmware.isEmpty()) {
        emit errorOccurred(tr("Firmware data is empty"));
        emit finished(false, tr("Empty firmware"));
        return;
    }

    emit statusMessage(tr("Starting firmware update..."));
    emit progressChanged(0);

    // --- Erase ---
    if (!doErase(startAddress, static_cast<quint32>(firmware.size()))) {
        emit finished(false, tr("Erase failed"));
        return;
    }
    if (m_abortRequested) {
        emit finished(false, tr("Update aborted"));
        return;
    }

    // --- Write ---
    if (!doWrite(firmware, startAddress, chunkSize)) {
        emit finished(false, tr("Write failed"));
        return;
    }
    if (m_abortRequested) {
        emit finished(false, tr("Update aborted"));
        return;
    }

    // --- Verify ---
    if (!doVerify(firmware, startAddress)) {
        emit finished(false, tr("Verify failed"));
        return;
    }
    if (m_abortRequested) {
        emit finished(false, tr("Update aborted"));
        return;
    }

    // --- Jump ---
    if (!doJump(startAddress)) {
        emit statusMessage(tr("Warning: Jump to application failed"));
    }

    emit progressChanged(100);
    emit finished(true, tr("Firmware update completed successfully"));
}

void FirmwareUpdateWorker::requestAbort()
{
    m_abortRequested = true;
    emit statusMessage(tr("Abort requested..."));
}

bool FirmwareUpdateWorker::doErase(quint32 address, quint32 size)
{
    emit statusMessage(tr("Erasing memory..."));
    emit progressChanged(5);

    if (!m_target->erase(address, size)) {
        emit errorOccurred(tr("Erase operation failed"));
        return false;
    }

    emit progressChanged(15);
    return true;
}

bool FirmwareUpdateWorker::doWrite(const QByteArray &firmware, quint32 startAddress, quint32 chunkSize)
{
    emit statusMessage(tr("Writing firmware..."));

    const quint32 totalSize = static_cast<quint32>(firmware.size());
    quint32 offset = 0;

    while (offset < totalSize) {
        if (m_abortRequested)
            return false;

        const quint32 remaining = totalSize - offset;
        const quint32 len = qMin(chunkSize, remaining);
        const QByteArray chunk = firmware.mid(offset, len);
        const quint32 address = startAddress + offset;

        if (!m_target->write(address, chunk)) {
            emit errorOccurred(tr("Write failed at address 0x%1")
                                   .arg(address, 8, 16, QChar('0')));
            return false;
        }

        offset += len;
        const int percent = 15 + static_cast<int>((offset * 70.0) / totalSize);
        emit progressChanged(percent);
    }

    emit progressChanged(85);
    return true;
}

bool FirmwareUpdateWorker::doVerify(const QByteArray &firmware, quint32 startAddress)
{
    emit statusMessage(tr("Verifying firmware..."));

    if (!m_target->verify(startAddress, firmware)) {
        emit errorOccurred(tr("Verification failed"));
        return false;
    }

    emit progressChanged(95);
    return true;
}

bool FirmwareUpdateWorker::doJump(quint32 address)
{
    emit statusMessage(tr("Jumping to application..."));
    return m_target->jumpTo(address);
}

} // namespace edcc
