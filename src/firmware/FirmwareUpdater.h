#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QThread>
#include "IMemoryTarget.h"

namespace edcc {

class FirmwareUpdateWorker;

/**
 * @brief High-level asynchronous firmware update engine.
 *
 * Runs the update process in a dedicated worker thread so the UI stays responsive.
 */
class FirmwareUpdater : public QObject
{
    Q_OBJECT

public:
    explicit FirmwareUpdater(IMemoryTarget *target, QObject *parent = nullptr);
    ~FirmwareUpdater() override;

    /**
     * @brief Start the firmware update process (non-blocking).
     * @return true if the update was successfully started
     */
    bool startUpdate(const QByteArray &firmwareData, quint32 startAddress);

    /**
     * @brief Request abort of the current update.
     */
    void abort();

    bool isBusy() const;

    void setChunkSize(quint32 size);

signals:
    void progressChanged(int percent);
    void finished(bool success, const QString &message);
    void errorOccurred(const QString &errorString);
    void statusMessage(const QString &message);

    // Internal signal to start the worker
    void startRequested(const QByteArray &firmware, quint32 startAddress, quint32 chunkSize);

private slots:
    void onWorkerFinished(bool success, const QString &message);
    void onWorkerProgress(int percent);
    void onWorkerStatus(const QString &message);
    void onWorkerError(const QString &error);

private:
    void setupWorker(IMemoryTarget *target);

    IMemoryTarget *m_target = nullptr;
    QThread *m_thread = nullptr;
    FirmwareUpdateWorker *m_worker = nullptr;

    quint32 m_chunkSize = 256;
    bool m_busy = false;
};

} // namespace edcc
