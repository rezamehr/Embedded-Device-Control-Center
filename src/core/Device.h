#pragma once

#include "IDevice.h"
#include "ICommunication.h"
#include "Types.h"

namespace edcc {

/**
 * @brief Concrete device that owns a communication channel
 *        and keeps its full configuration (DeviceInfo).
 */
class Device : public IDevice
{
    Q_OBJECT

public:
    /**
     * @param info           Full device configuration
     * @param communication  Ownership is transferred to this Device
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
     * @brief Send raw data through the underlying communication channel.
     */
    qint64 send(const QByteArray &data);

    /**
     * @brief Returns the full configuration of this device.
     *        Used for saving to JSON.
     */
    DeviceInfo info() const;

private slots:
    void onCommStateChanged(ConnectionState state);
    void onCommDataReceived(const QByteArray &data);
    void onCommError(const QString &error);

private:
    DeviceInfo m_info;                 // Full configuration (kept for save/load)
    ICommunication *m_comm = nullptr;  // Owned by this object
    ConnectionState m_state = ConnectionState::Disconnected;
};

} // namespace edcc
