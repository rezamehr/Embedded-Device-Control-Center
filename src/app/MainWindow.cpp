#include "MainWindow.h"

#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

namespace edcc {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_deviceManager = new DeviceManager(this);

    setupUi();
    onRefreshPorts();
    setConnectedState(false);
    onConnectionTypeChanged(0); // Default to Serial

    // Connect DeviceManager signals
    connect(m_deviceManager, &DeviceManager::deviceAdded,
            this, &MainWindow::onDeviceAdded);
    connect(m_deviceManager, &DeviceManager::deviceRemoved,
            this, &MainWindow::onDeviceRemoved);
    connect(m_deviceManager, &DeviceManager::deviceStateChanged,
            this, &MainWindow::onDeviceStateChanged);
    connect(m_deviceManager, &DeviceManager::deviceDataReceived,
            this, &MainWindow::onDeviceDataReceived);
    connect(m_deviceManager, &DeviceManager::deviceError,
            this, &MainWindow::onDeviceError);

    setWindowTitle("Embedded Device Control Center");
    resize(1100, 700);
}

MainWindow::~MainWindow()
{
    clearCurrentConnection();
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QHBoxLayout(central);

    // ==================== LEFT PANEL: Device List ====================
    auto *leftPanel = new QVBoxLayout();

    leftPanel->addWidget(new QLabel("Devices"));
    m_deviceList = new QListWidget();
    m_deviceList->setMaximumWidth(230);
    leftPanel->addWidget(m_deviceList, 1);

    m_btnAddDevice = new QPushButton("Add Device");
    m_btnRemoveDevice = new QPushButton("Remove Device");
    leftPanel->addWidget(m_btnAddDevice);
    leftPanel->addWidget(m_btnRemoveDevice);

    // ==================== RIGHT PANEL ====================
    auto *rightPanel = new QVBoxLayout();

    // --- Connection settings bar ---
    auto *topLayout = new QHBoxLayout();

    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({"Serial", "TCP"});

    // Serial controls
    m_serialControls = new QWidget();
    auto *serialLayout = new QHBoxLayout(m_serialControls);
    serialLayout->setContentsMargins(0, 0, 0, 0);

    m_portCombo = new QComboBox();
    m_baudCombo = new QComboBox();
    m_baudCombo->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    m_baudCombo->setCurrentText("115200");
    m_btnRefresh = new QPushButton("Refresh");

    serialLayout->addWidget(new QLabel("Port:"));
    serialLayout->addWidget(m_portCombo, 1);
    serialLayout->addWidget(new QLabel("Baud:"));
    serialLayout->addWidget(m_baudCombo);
    serialLayout->addWidget(m_btnRefresh);

    // TCP controls
    m_tcpControls = new QWidget();
    auto *tcpLayout = new QHBoxLayout(m_tcpControls);
    tcpLayout->setContentsMargins(0, 0, 0, 0);

    m_hostEdit = new QLineEdit("127.0.0.1");
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(12345);

    tcpLayout->addWidget(new QLabel("Host:"));
    tcpLayout->addWidget(m_hostEdit, 1);
    tcpLayout->addWidget(new QLabel("Port:"));
    tcpLayout->addWidget(m_portSpin);

    m_btnConnect = new QPushButton("Connect");

    topLayout->addWidget(new QLabel("Type:"));
    topLayout->addWidget(m_typeCombo);
    topLayout->addWidget(m_serialControls, 1);
    topLayout->addWidget(m_tcpControls, 1);
    topLayout->addWidget(m_btnConnect);

    // --- Tool buttons ---
    auto *toolLayout = new QHBoxLayout();
    m_btnClear = new QPushButton("Clear Log");
    m_btnSaveLog = new QPushButton("Save Log");
    toolLayout->addStretch();
    toolLayout->addWidget(m_btnClear);
    toolLayout->addWidget(m_btnSaveLog);

    // --- Log view ---
    m_logView = new QTextEdit();
    m_logView->setReadOnly(true);
    m_logView->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #d4d4d4; font-family: Consolas; font-size: 13px; }"
    );

    // --- Send area ---
    auto *sendLayout = new QHBoxLayout();
    m_sendEdit = new QLineEdit();
    m_btnSend = new QPushButton("Send");
    sendLayout->addWidget(m_sendEdit, 1);
    sendLayout->addWidget(m_btnSend);

    m_statusLabel = new QLabel("Status: Ready");

    rightPanel->addLayout(topLayout);
    rightPanel->addLayout(toolLayout);
    rightPanel->addWidget(m_logView, 1);
    rightPanel->addLayout(sendLayout);
    rightPanel->addWidget(m_statusLabel);

    // Add panels to main layout
    mainLayout->addLayout(leftPanel);
    mainLayout->addLayout(rightPanel, 1);

    // ==================== Connections ====================
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onConnectionTypeChanged);
    connect(m_btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshPorts);
    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_btnAddDevice, &QPushButton::clicked, this, &MainWindow::onAddDeviceClicked);
    connect(m_btnRemoveDevice, &QPushButton::clicked, this, &MainWindow::onRemoveDeviceClicked);
    connect(m_deviceList, &QListWidget::itemSelectionChanged, this, &MainWindow::onDeviceSelected);
    connect(m_btnSend, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(m_sendEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);
    connect(m_btnClear, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(m_btnSaveLog, &QPushButton::clicked, this, &MainWindow::onSaveLogClicked);
}

void MainWindow::onConnectionTypeChanged(int index)
{
    bool isSerial = (index == 0);
    m_serialControls->setVisible(isSerial);
    m_tcpControls->setVisible(!isSerial);
}

void MainWindow::onRefreshPorts()
{
    m_portCombo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        QString label = info.portName();
        if (!info.description().isEmpty())
            label += " - " + info.description();
        m_portCombo->addItem(label, info.portName());
    }
    if (m_portCombo->count() == 0)
        m_portCombo->addItem("No ports found");
}

ICommunication* MainWindow::createCommunication()
{
    const bool isSerial = (m_typeCombo->currentIndex() == 0);

    if (isSerial) {
        if (m_portCombo->currentData().isNull())
            return nullptr;

        QString portName = m_portCombo->currentData().toString();
        qint32 baud = m_baudCombo->currentText().toInt();
        return new SerialCommunication(portName, baud);
    } else {
        QString host = m_hostEdit->text().trimmed();
        if (host.isEmpty())
            return nullptr;

        quint16 port = static_cast<quint16>(m_portSpin->value());
        return new TcpCommunication(host, port);
    }
}

void MainWindow::onAddDeviceClicked()
{
    ICommunication *comm = createCommunication();
    if (!comm) {
        QMessageBox::warning(this, "Warning", "Invalid connection settings.");
        return;
    }

    DeviceInfo info;
    info.name = comm->name();
    info.description = (m_typeCombo->currentIndex() == 0) ? "Serial Device" : "TCP Device";

    QString id = m_deviceManager->addDevice(info, comm);
    if (id.isEmpty()) {
        delete comm;
        QMessageBox::warning(this, "Error", "Failed to add device.");
        return;
    }

    appendLog("Device added: " + id + " (" + info.name + ")", "#3fb950");
}

void MainWindow::onRemoveDeviceClicked()
{
    auto *item = m_deviceList->currentItem();
    if (!item) {
        QMessageBox::information(this, "Info", "Please select a device to remove.");
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    m_deviceManager->removeDevice(id);
}

void MainWindow::onDeviceSelected()
{
    auto *item = m_deviceList->currentItem();
    if (!item) {
        m_currentDeviceId.clear();
        return;
    }
    m_currentDeviceId = item->data(Qt::UserRole).toString();
    updateStatusLabel();
}

void MainWindow::onConnectClicked()
{
    if (m_currentDeviceId.isEmpty()) {
        QMessageBox::information(this, "Info", "Please select a device from the list first.\n"
                                               "Use 'Add Device' to register a new device.");
        return;
    }

    Device *dev = m_deviceManager->device(m_currentDeviceId);
    if (!dev) return;

    if (dev->isConnected()) {
        dev->disconnectFromDevice();
    } else {
        appendLog("Connecting " + m_currentDeviceId + " ...", "#aaaaaa");
        dev->connectToDevice();
    }
}

void MainWindow::onSendClicked()
{
    if (m_currentDeviceId.isEmpty()) {
        QMessageBox::information(this, "Info", "No device selected.");
        return;
    }

    Device *dev = m_deviceManager->device(m_currentDeviceId);
    if (!dev || !dev->isConnected()) {
        appendLog("Device is not connected", "#f85149");
        return;
    }

    QString text = m_sendEdit->text();
    if (text.isEmpty()) return;

    QByteArray data = (text + "\r\n").toUtf8();
    qint64 written = dev->send(data);

    if (written > 0) {
        m_txBytes += written;
        appendLog(QString("[%1] TX: %2").arg(m_currentDeviceId, text), "#3fb950");
        updateStatusLabel();
        m_sendEdit->clear();
    }
}

void MainWindow::onClearClicked()
{
    m_logView->clear();
    m_txBytes = 0;
    m_rxBytes = 0;
    updateStatusLabel();
}

void MainWindow::onSaveLogClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Log",
        QString("edcc_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        "Text Files (*.txt)"
    );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_logView->toPlainText();
        appendLog("Log saved to " + fileName, "#3fb950");
    } else {
        appendLog("Failed to save log", "#f85149");
    }
}

// ==================== DeviceManager callbacks ====================

void MainWindow::onDeviceAdded(const QString &id)
{
    Device *dev = m_deviceManager->device(id);
    if (!dev) return;

    auto *item = new QListWidgetItem(dev->name() + "  [" + id + "]");
    item->setData(Qt::UserRole, id);
    m_deviceList->addItem(item);
    m_deviceList->setCurrentItem(item);
}

void MainWindow::onDeviceRemoved(const QString &id)
{
    for (int i = 0; i < m_deviceList->count(); ++i) {
        QListWidgetItem *item = m_deviceList->item(i);
        if (item->data(Qt::UserRole).toString() == id) {
            delete m_deviceList->takeItem(i);
            break;
        }
    }

    if (m_currentDeviceId == id) {
        m_currentDeviceId.clear();
    }

    appendLog("Device removed: " + id, "#f85149");
}

void MainWindow::onDeviceStateChanged(const QString &id, ConnectionState state)
{
    QString msg;
    QString color;

    switch (state) {
    case ConnectionState::Connected:
        msg = "Connected";
        color = "#3fb950";
        setConnectedState(true);
        break;
    case ConnectionState::Disconnected:
        msg = "Disconnected";
        color = "#f85149";
        setConnectedState(false);
        break;
    case ConnectionState::Connecting:
        msg = "Connecting...";
        color = "#aaaaaa";
        break;
    case ConnectionState::Error:
        msg = "Error";
        color = "#f85149";
        setConnectedState(false);
        break;
    }

    appendLog(QString("[%1] %2").arg(id, msg), color);
    updateStatusLabel();
}

void MainWindow::onDeviceDataReceived(const QString &id, const QByteArray &data)
{
    QString text = QString::fromUtf8(data).trimmed();
    if (text.isEmpty()) return;

    m_rxBytes += data.size();
    appendLog(QString("[%1] RX: %2").arg(id, text), "#58a6ff");
    updateStatusLabel();
}

void MainWindow::onDeviceError(const QString &id, const QString &error)
{
    appendLog(QString("[%1] Error: %2").arg(id, error), "#f85149");
}

// ==================== Helpers ====================

void MainWindow::setConnectedState(bool connected)
{
    m_btnConnect->setText(connected ? "Disconnect" : "Connect");
    m_btnSend->setEnabled(connected);
    m_sendEdit->setEnabled(connected);
}

void MainWindow::appendLog(const QString &text, const QString &color)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    m_logView->append(
        QString("<span style='color:gray;'>[%1]</span> <span style='color:%2;'>%3</span>")
            .arg(time, color, text.toHtmlEscaped())
    );
}

void MainWindow::updateStatusLabel()
{
    if (m_currentDeviceId.isEmpty()) {
        m_statusLabel->setText(QString("Status: Ready | Devices: %1 | TX: %2  RX: %3")
                                   .arg(m_deviceManager->deviceCount())
                                   .arg(m_txBytes)
                                   .arg(m_rxBytes));
    } else {
        Device *dev = m_deviceManager->device(m_currentDeviceId);
        QString stateStr = (dev && dev->isConnected()) ? "Connected" : "Disconnected";
        m_statusLabel->setText(QString("Status: %1 [%2] | TX: %3  RX: %4")
                                   .arg(stateStr, m_currentDeviceId)
                                   .arg(m_txBytes)
                                   .arg(m_rxBytes));
    }
}

void MainWindow::clearCurrentConnection()
{
    if (m_tempComm) {
        m_tempComm->close();
        m_tempComm->deleteLater();
        m_tempComm = nullptr;
    }
}

} // namespace edcc
