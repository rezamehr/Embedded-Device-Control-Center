#include "FirmwareUpdater.h"
#include "FirmwareUpdateWorker.h"
#include "logging/Logger.h"

namespace edcc {

FirmwareUpdater::FirmwareUpdater(IMemoryTarget *target, QObject *parent)
    : QObject(parent)
    , m_target(target)
{
    Q_ASSERT(m_target != nullptr);
    setupWorker(target);
}

FirmwareUpdater::~FirmwareUpdater()
{
    if (m_thread) {
        m_thread->quit();
        m_thread->wait(3000);
    }
}

void FirmwareUpdater::setupWorker(IMemoryTarget *target)
{
    m_thread = new QThread(this);
    m_worker = new FirmwareUpdateWorker(target);

    m_worker->moveToThread(m_thread);

    // Forward signals from worker to this object
    connect(m_worker, &FirmwareUpdateWorker::progressChanged,
            this, &FirmwareUpdater::onWorkerProgress);
    connect(m_worker, &FirmwareUpdateWorker::statusMessage,
            this, &FirmwareUpdater::onWorkerStatus);
    connect(m_worker, &FirmwareUpdateWorker::errorOccurred,
            this, &FirmwareUpdater::onWorkerError);
    connect(m_worker, &FirmwareUpdateWorker::finished,
            this, &FirmwareUpdater::onWorkerFinished);

    // Start request
    connect(this, &FirmwareUpdater::startRequested,
            m_worker, &FirmwareUpdateWorker::doUpdate);

    // Cleanup
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_thread->start();
}

bool FirmwareUpdater::startUpdate(const QByteArray &firmwareData, quint32 startAddress)
{
    if (m_busy) {
        emit errorOccurred(tr("Update already in progress"));
        return false;
    }

    if (firmwareData.isEmpty()) {
        emit errorOccurred(tr("Firmware data is empty"));
        return false;
    }

    m_busy = true;
    emit startRequested(firmwareData, startAddress, m_chunkSize);
    return true;
}

void FirmwareUpdater::abort()
{
    if (m_busy && m_worker) {
        // Direct call is safe because requestAbort only sets a flag
        QMetaObject::invokeMethod(m_worker, "requestAbort", Qt::QueuedConnection);
    }
}

bool FirmwareUpdater::isBusy() const
{
    return m_busy;
}

void FirmwareUpdater::setChunkSize(quint32 size)
{
    if (size > 0)
        m_chunkSize = size;
}

void FirmwareUpdater::onWorkerFinished(bool success, const QString &message)
{
    m_busy = false;
    emit finished(success, message);

    if (success)
        Logger::instance().info(QStringLiteral("FirmwareUpdater"), message);
    else
        Logger::instance().error(QStringLiteral("FirmwareUpdater"), message);
}

void FirmwareUpdater::onWorkerProgress(int percent)
{
    emit progressChanged(percent);
}

void FirmwareUpdater::onWorkerStatus(const QString &message)
{
    emit statusMessage(message);
}

void FirmwareUpdater::onWorkerError(const QString &error)
{
    emit errorOccurred(error);
}

} // namespace edcc
