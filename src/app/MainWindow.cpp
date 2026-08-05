#include "MainWindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>

namespace edcc {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    onRefreshPorts();
    setConnectedState(false);
    onConnectionTypeChanged(0);   // default = Serial

    setWindowTitle("Embedded Device Control Center");
    resize(950, 680);
}

MainWindow::~MainWindow()
{
    clearCurrentConnection();
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);

    // === Top controls ===
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

    // Log
    m_logView = new QTextEdit();
    m_logView->setReadOnly(true);
    m_logView->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; font-family: Consolas; font-size: 13px;");

    // Send area
    auto *sendLayout = new QHBoxLayout();
    m_sendEdit = new QLineEdit();
    m_btnSend = new QPushButton("Send");
    sendLayout->addWidget(m_sendEdit, 1);
    sendLayout->addWidget(m_btnSend);

    m_statusLabel = new QLabel("Status: Disconnected");

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_logView, 1);
    mainLayout->addLayout(sendLayout);
    mainLayout->addWidget(m_statusLabel);

    // Signals
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onConnectionTypeChanged);
    connect(m_btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshPorts);
    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_btnSend, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(m_sendEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);
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

void MainWindow::setConnectedState(bool connected)
{
    m_typeCombo->setEnabled(!connected);
    m_serialControls->setEnabled(!connected);
    m_tcpControls->setEnabled(!connected);
    m_btnSend->setEnabled(connected);
    m_sendEdit->setEnabled(connected);
    m_btnConnect->setText(connected ? "Disconnect" : "Connect");
}

void MainWindow::clearCurrentConnection()
{
    if (m_comm) {
        m_comm->close();
        m_comm->deleteLater();
        m_comm = nullptr;
    }
}

void MainWindow::onConnectClicked()
{
    // Disconnect if already connected
    if (m_comm && m_comm->isOpen()) {
        clearCurrentConnection();
        setConnectedState(false);
        m_statusLabel->setText("Status: Disconnected");
        m_logView->append("<span style='color:#f85149;'>Disconnected</span>");
        return;
    }

    clearCurrentConnection();

    const bool isSerial = (m_typeCombo->currentIndex() == 0);

    if (isSerial) {
        if (m_portCombo->currentData().isNull()) {
            QMessageBox::warning(this, "Warning", "No valid serial port selected.");
            return;
        }
        QString portName = m_portCombo->currentData().toString();
        qint32 baud = m_baudCombo->currentText().toInt();
        m_comm = new SerialCommunication(portName, baud, this);
    } else {
        QString host = m_hostEdit->text().trimmed();
        if (host.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Host cannot be empty.");
            return;
        }
        quint16 port = static_cast<quint16>(m_portSpin->value());
        m_comm = new TcpCommunication(host, port, this);
    }

    connect(m_comm, &ICommunication::dataReceived, this, &MainWindow::onDataReceived);
    connect(m_comm, &ICommunication::stateChanged, this, &MainWindow::onStateChanged);
    connect(m_comm, &ICommunication::errorOccurred, this, &MainWindow::onErrorOccurred);

    m_logView->append("<span style='color:#aaaaaa;'>Connecting via " + m_comm->name() + " ...</span>");
    m_comm->open();
}

void MainWindow::onSendClicked()
{
    if (!m_comm || !m_comm->isOpen())
        return;

    QString text = m_sendEdit->text();
    if (text.isEmpty())
        return;

    QByteArray data = (text + "\r\n").toUtf8();
    m_comm->send(data);

    m_logView->append("<span style='color:#3fb950;'>TX: " + text.toHtmlEscaped() + "</span>");
    m_sendEdit->clear();
}

void MainWindow::onDataReceived(const QByteArray &data)
{
    QString text = QString::fromUtf8(data).trimmed();
    if (!text.isEmpty()) {
        m_logView->append("<span style='color:#58a6ff;'>RX: " + text.toHtmlEscaped() + "</span>");
    }
}

void MainWindow::onStateChanged(edcc::ConnectionState state)
{
    switch (state) {
    case ConnectionState::Connected:
        setConnectedState(true);
        m_statusLabel->setText("Status: Connected (" + m_comm->name() + ")");
        m_logView->append("<span style='color:#3fb950;'>Connected</span>");
        break;
    case ConnectionState::Disconnected:
        setConnectedState(false);
        m_statusLabel->setText("Status: Disconnected");
        break;
    case ConnectionState::Connecting:
        m_statusLabel->setText("Status: Connecting...");
        break;
    case ConnectionState::Error:
        setConnectedState(false);
        m_statusLabel->setText("Status: Error");
        break;
    }
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_logView->append("<span style='color:#f85149;'>Error: " + error.toHtmlEscaped() + "</span>");
}

} // namespace edcc
