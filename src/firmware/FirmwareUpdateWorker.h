#pragma once

#include <QObject>
#include <QByteArray>
#include "IMemoryTarget.h"

namespace edcc {

/**
 * @brief Performs the actual firmware update steps in a background thread.
 */
class FirmwareUpdateWorker : public QObject
{
    Q_OBJECT

public:
    explicit FirmwareUpdateWorker(IMemoryTarget *target, QObject *parent = nullptr);

public slots:
    void doUpdate(const QByteArray &firmware, quint32 startAddress, quint32 chunkSize);
    void requestAbort();

signals:
    void progressChanged(int percent);
    void statusMessage(const QString &message);
    void errorOccurred(const QString &errorString);
    void finished(bool success, const QString &message);

private:
    bool doErase(quint32 address, quint32 size);
    bool doWrite(const QByteArray &firmware, quint32 startAddress, quint32 chunkSize);
    bool doVerify(const QByteArray &firmware, quint32 startAddress);
    bool doJump(quint32 address);

    IMemoryTarget *m_target = nullptr;
    bool m_abortRequested = false;
};

} // namespace edcc
