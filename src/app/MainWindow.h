#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QCheckBox>
#include <QProgressBar>
#include <QGroupBox>
#include <QFile>

#include "core/DeviceManager.h"
#include "logging/Logger.h"
#include "communication/SerialCommunication.h"
#include "communication/TcpCommunication.h"
#include "firmware/FirmwareUpdater.h"
#include "firmware/DummyMemoryTarget.h"

namespace edcc {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectionTypeChanged(int index);
    void onRefreshPorts();
    void onConnectClicked();          // Connect / Disconnect selected or new device
    void onAddDeviceClicked();        // Add current connection settings as a device
    void onRemoveDeviceClicked();
    void onDeviceSelected();
    void onSendClicked();
    void onClearClicked();
    void onSaveLogClicked();
    void updateDeviceListItem(const QString &id);

    // DeviceManager signals
    void onDeviceAdded(const QString &id);
    void onDeviceRemoved(const QString &id);
    void onDeviceStateChanged(const QString &id, edcc::ConnectionState state);
    void onDeviceDataReceived(const QString &id, const QByteArray &data);
    void onDeviceError(const QString &id, const QString &error);

    //Firmware
    void onBrowseFirmware();
    void onStartFirmwareUpdate();
    void onAbortFirmwareUpdate();
    void onFirmwareProgress(int percent);
    void onFirmwareStatus(const QString &message);
    void onFirmwareFinished(bool success, const QString &message);
    void onFirmwareError(const QString &error);
private:
    void setupUi();
    void setConnectedState(bool connected);
    //void appendLog(const QString &text, const QString &color);
    void updateStatusLabel();
    void clearCurrentConnection();
    ICommunication* createCommunication();   // Create Serial or TCP based on UI

    // Left panel - Device list
    QListWidget *m_deviceList;
    QPushButton *m_btnAddDevice;
    QPushButton *m_btnRemoveDevice;

    // Connection settings
    QComboBox *m_typeCombo;
    QComboBox *m_portCombo;
    QComboBox *m_baudCombo;
    QLineEdit *m_hostEdit;
    QSpinBox  *m_portSpin;
    QPushButton *m_btnRefresh;
    QPushButton *m_btnConnect;

    QCheckBox *m_chkShowRaw = nullptr;

    QWidget *m_serialControls;
    QWidget *m_tcpControls;

    // Log and send
    QTextEdit *m_logView;
    QLineEdit *m_sendEdit;
    QPushButton *m_btnSend;
    QPushButton *m_btnClear;
    QPushButton *m_btnSaveLog;
    QLabel *m_statusLabel;

    // Core
    DeviceManager *m_deviceManager = nullptr;
    QString m_currentDeviceId;          // Currently selected device ID
    ICommunication *m_tempComm = nullptr; // Temporary connection before adding to manager

    qint64 m_txBytes = 0;
    qint64 m_rxBytes = 0;

    // Firmware Update UI
    QGroupBox   *m_firmwareGroup = nullptr;
    QLineEdit   *m_fwPathEdit = nullptr;
    QPushButton *m_btnBrowseFw = nullptr;
    QLineEdit   *m_fwAddressEdit = nullptr;
    QProgressBar *m_fwProgress = nullptr;
    QLabel      *m_fwStatusLabel = nullptr;
    QPushButton *m_btnStartUpdate = nullptr;
    QPushButton *m_btnAbortUpdate = nullptr;

    // Firmware engine (Dummy for now)
    DummyMemoryTarget *m_dummyTarget = nullptr;
    FirmwareUpdater   *m_fwUpdater = nullptr;
    QByteArray         m_firmwareData;
};

} // namespace edcc
