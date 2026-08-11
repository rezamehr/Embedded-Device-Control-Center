#pragma once

#include "IMemoryTarget.h"
#include "core/ICommunication.h"

#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>

namespace edcc {

/**
 * @brief IMemoryTarget implementation that communicates with a custom
 *        bootloader over an existing ICommunication channel (Serial/TCP).
 *
 * Typical usage:
 *   1. Open the communication channel
 *   2. Call queryIdentity()
 *   3. Use erase / write / read / verify / jumpTo through FirmwareUpdater
 */
class SerialBootloaderTarget : public IMemoryTarget
{
    Q_OBJECT

public:
    explicit SerialBootloaderTarget(ICommunication *communication,
                                    QObject *parent = nullptr);
    ~SerialBootloaderTarget() override;

    // IMemoryTarget
    MemoryTargetInfo info() const override;
    bool isReady() const override;

    bool erase(quint32 address, quint32 size) override;
    bool massErase() override;

    bool write(quint32 address, const QByteArray &data) override;
    QByteArray read(quint32 address, quint32 size) override;
    bool verify(quint32 address, const QByteArray &expected) override;

    bool jumpTo(quint32 address) override;

    /**
     * @brief Query device identity and fill MemoryTargetInfo.
     * @return true on success
     */
    bool queryIdentity();

private slots:
    void onDataReceived(const QByteArray &data);

private:
    /**
     * @brief Send a frame and wait for a complete response payload.
     */
    bool sendCommandAndWait(const QByteArray &frame,
                            QByteArray &responsePayload,
                            int timeoutMs = 2000);

    ICommunication *m_comm = nullptr;
    MemoryTargetInfo m_info;
    bool m_ready = false;

    QMutex m_mutex;
    QWaitCondition m_condition;
    QByteArray m_receiveBuffer;
    QByteArray m_responsePayload;
    bool m_responseReady = false;
};

} // namespace edcc
