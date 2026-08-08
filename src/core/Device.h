#pragma once

#include "IDevice.h"
#include "ICommunication.h"
#include "Types.h"
#include <QThread>

namespace edcc {

class CommunicationWorker;

/**
 * @brief Represents a managed device.
 *
 * Owns a CommunicationWorker that runs in its own QThread.
 * All I/O is performed off the UI thread.
 */
class Device : public IDevice
{
    Q_OBJECT

public:
    /**
     * @param info           Full device configuration (used for save/load)
     * @param communication  Ownership is transferred. Will be moved to worker thread.
     */
    explicit Device(const DeviceInfo &info,
                    ICommunication *communication,
                    QObject *parent = nullptr);
    ~Device() override;

    // IDevice interface
    QString id() const override;
    QString name() const override;
    QString description() const override;

    bool connectToDevice() override;
    void disconnectFromDevice() override;
    bool isConnected() const override;
    ConnectionState state() const override;

    /**
     * @brief Send data to the device (thread-safe).
     */
    qint64 send(const QByteArray &data);

    /**
     * @brief Returns the full configuration of this device.
     */
    DeviceInfo info() const;

signals:
    // Re-declare so external code can connect easily
    void stateChanged(edcc::ConnectionState state);
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &errorString);

private slots:
    void onWorkerStateChanged(edcc::ConnectionState state);
    void onWorkerDataReceived(const QByteArray &data);
    void onWorkerError(const QString &error);
    void onWorkerBytesWritten(qint64 bytes);

private:
    void setupWorker(ICommunication *communication);

    DeviceInfo m_info;
    ConnectionState m_state = ConnectionState::Disconnected;

    QThread *m_thread = nullptr;
    CommunicationWorker *m_worker = nullptr;

    // We cannot easily return the real written size from another thread,
    // so send() becomes fire-and-forget from the caller's point of view.
    // Actual write confirmation comes via bytesWritten if needed later.
};

} // namespace edcc
