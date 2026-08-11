#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include "IMemoryTarget.h"

namespace edcc {

/**
 * @brief High-level firmware update engine.
 *
 * Orchestrates the complete update process using an IMemoryTarget:
 * - Erase
 * - Write (in chunks)
 * - Verify
 * - Jump to application
 *
 * This class is independent of the underlying flash technology.
 */
class FirmwareUpdater : public QObject
{
    Q_OBJECT

public:
    explicit FirmwareUpdater(IMemoryTarget *target, QObject *parent = nullptr);
    ~FirmwareUpdater() override = default;

    /**
     * @brief Start the firmware update process.
     * @param firmwareData  Complete firmware binary
     * @param startAddress  Address where the firmware should be written
     * @return true if the update process was started successfully
     */
    bool startUpdate(const QByteArray &firmwareData, quint32 startAddress);

    /**
     * @brief Abort the current update process (if possible).
     */
    void abort();

    /**
     * @brief Returns whether an update is currently in progress.
     */
    bool isBusy() const;

    /**
     * @brief Set the write chunk size (default is taken from MemoryTargetInfo).
     */
    void setChunkSize(quint32 size);

signals:
    /**
     * @brief Emitted during the update process (0..100).
     */
    void progressChanged(int percent);

    /**
     * @brief Emitted when the entire update finishes.
     * @param success  true if update completed successfully
     * @param message  Human readable result message
     */
    void finished(bool success, const QString &message);


    /**
     * @brief Emitted when an error occurs during the update.
     */
    void errorOccurred(const QString &errorString);

    /**
     * @brief Detailed status messages for logging / UI.
     */
    void statusMessage(const QString &message);

private:
    IMemoryTarget *m_target = nullptr;
    QByteArray m_firmware;
    quint32 m_startAddress = 0;
    quint32 m_chunkSize = 256;
    bool m_busy = false;
    bool m_abortRequested = false;

    // Internal helper methods (will be implemented later)
    bool doErase();
    bool doWrite();
    bool doVerify();
    bool doJump();
    void finish(bool success, const QString &message);
};

} // namespace edcc
