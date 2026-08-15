#pragma once

#include "IMemoryTarget.h"
#include "core/Device.h"

#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>

namespace edcc {

/**
 * @brief Bootloader memory target bound to an existing managed Device.
 *
 * Uses Device::send() and Device::dataReceived for transport.
 * The device must already be connected.
 */
class SerialBootloaderTarget : public IMemoryTarget
{
    Q_OBJECT

public:
    explicit SerialBootloaderTarget(Device *device, QObject *parent = nullptr);
    ~SerialBootloaderTarget() override;

    MemoryTargetInfo info() const override;
    bool isReady() const override;

    bool erase(quint32 address, quint32 size) override;
    bool massErase() override;

    bool write(quint32 address, const QByteArray &data) override;
    QByteArray read(quint32 address, quint32 size) override;
    bool verify(quint32 address, const QByteArray &expected) override;
    bool jumpTo(quint32 address) override;

    bool queryIdentity();

private slots:
    void onDeviceDataReceived(const QByteArray &data);
signals:
    void responseReceived();
private:
    bool sendCommandAndWait(const QByteArray &frame,
                            QByteArray &responsePayload,
                            int timeoutMs = 2000);

    Device *m_device = nullptr;
    MemoryTargetInfo m_info;
    bool m_ready = false;

    QMutex m_mutex;
    QWaitCondition m_condition;
    QByteArray m_receiveBuffer;
    QByteArray m_responsePayload;
    bool m_responseReady = false;
};

} // namespace edcc
