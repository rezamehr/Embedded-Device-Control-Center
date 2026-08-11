#include "FirmwareUpdater.h"
#include "logging/Logger.h"

namespace edcc {

FirmwareUpdater::FirmwareUpdater(IMemoryTarget *target, QObject *parent)
    : QObject(parent)
    , m_target(target)
{
    Q_ASSERT(m_target != nullptr);

    // Forward signals from the target (optional but useful)
    connect(m_target, &IMemoryTarget::progressChanged,
            this, &FirmwareUpdater::progressChanged);

    connect(m_target, &IMemoryTarget::errorOccurred,
            this, &FirmwareUpdater::errorOccurred);
}

bool FirmwareUpdater::startUpdate(const QByteArray &firmwareData, quint32 startAddress)
{
    if (m_busy) {
        emit errorOccurred(tr("Update already in progress"));
        return false;
    }

    if (!m_target || !m_target->isReady()) {
        emit errorOccurred(tr("Memory target is not ready"));
        return false;
    }

    if (firmwareData.isEmpty()) {
        emit errorOccurred(tr("Firmware data is empty"));
        return false;
    }

    m_firmware = firmwareData;
    m_startAddress = startAddress;
    m_busy = true;
    m_abortRequested = false;

    // Use recommended chunk size from target if available
    const auto info = m_target->info();
    if (info.maxWriteChunk > 0) {
        m_chunkSize = info.maxWriteChunk;
    }

    emit statusMessage(tr("Starting firmware update..."));
    emit progressChanged(0);

    // --- Step 1: Erase ---
    if (!doErase()) {
        finish(false, tr("Erase failed"));
        return false;
    }

    if (m_abortRequested) {
        finish(false, tr("Update aborted by user"));
        return false;
    }

    // --- Step 2: Write ---
    if (!doWrite()) {
        finish(false, tr("Write failed"));
        return false;
    }

    if (m_abortRequested) {
        finish(false, tr("Update aborted by user"));
        return false;
    }

    // --- Step 3: Verify ---
    if (!doVerify()) {
        finish(false, tr("Verify failed"));
        return false;
    }

    if (m_abortRequested) {
        finish(false, tr("Update aborted by user"));
        return false;
    }

    // --- Step 4: Jump (optional) ---
    // We can make jump optional later. For now we call it.
    if (!doJump()) {
        // Jump failure is not always critical, so we still report success of update
        emit statusMessage(tr("Warning: Jump to application failed"));
    }

    finish(true, tr("Firmware update completed successfully"));
    return true;
}

void FirmwareUpdater::abort()
{
    if (m_busy) {
        m_abortRequested = true;
        emit statusMessage(tr("Abort requested..."));
    }
}

bool FirmwareUpdater::isBusy() const
{
    return m_busy;
}

void FirmwareUpdater::setChunkSize(quint32 size)
{
    if (size > 0) {
        m_chunkSize = size;
    }
}

// ==================== Private helpers ====================

bool FirmwareUpdater::doErase()
{
    emit statusMessage(tr("Erasing memory..."));
    emit progressChanged(5);

    const bool ok = m_target->erase(m_startAddress, static_cast<quint32>(m_firmware.size()));
    if (!ok) {
        emit errorOccurred(tr("Erase operation failed"));
        return false;
    }

    emit progressChanged(15);
    return true;
}

bool FirmwareUpdater::doWrite()
{
    emit statusMessage(tr("Writing firmware..."));

    const quint32 totalSize = static_cast<quint32>(m_firmware.size());
    quint32 offset = 0;

    while (offset < totalSize) {
        if (m_abortRequested) {
            return false;
        }

        const quint32 remaining = totalSize - offset;
        const quint32 chunkLen = qMin(m_chunkSize, remaining);

        const QByteArray chunk = m_firmware.mid(offset, chunkLen);
        const quint32 address = m_startAddress + offset;

        if (!m_target->write(address, chunk)) {
            emit errorOccurred(tr("Write failed at address 0x%1").arg(address, 8, 16, QChar('0')));
            return false;
        }

        offset += chunkLen;

        // Progress: 15% → 85%
        const int percent = 15 + static_cast<int>((offset * 70.0) / totalSize);
        emit progressChanged(percent);
    }

    emit progressChanged(85);
    return true;
}

bool FirmwareUpdater::doVerify()
{
    emit statusMessage(tr("Verifying firmware..."));

    const bool ok = m_target->verify(m_startAddress, m_firmware);
    if (!ok) {
        emit errorOccurred(tr("Verification failed"));
        return false;
    }

    emit progressChanged(95);
    return true;
}

bool FirmwareUpdater::doJump()
{
    emit statusMessage(tr("Jumping to application..."));
    return m_target->jumpTo(m_startAddress);
}

void FirmwareUpdater::finish(bool success, const QString &message)
{
    m_busy = false;
    m_abortRequested = false;
    m_firmware.clear();

    emit progressChanged(success ? 100 : 0);
    emit finished(success, message);
    emit statusMessage(message);

    if (success) {
        Logger::instance().info(QStringLiteral("FirmwareUpdater"), message);
    } else {
        Logger::instance().error(QStringLiteral("FirmwareUpdater"), message);
    }
}

} // namespace edcc
