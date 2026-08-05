#include "MainWindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDateTime>

namespace edcc {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    onRefreshPorts();
    setConnectedState(false);

    setWindowTitle("Embedded Device Control Center");
    resize(900, 650);
}

MainWindow::~MainWindow()
{
    if (m_serial) {
        m_serial->close();
        m_serial->deleteLater();
    }
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    // Top bar
    auto *topLayout = new QHBoxLayout();
    m_portCombo = new QComboBox();
    m_baudCombo = new QComboBox();
    m_baudCombo->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    m_baudCombo->setCurrentText("115200");

    m_btnRefresh = new QPushButton("Refresh");
    m_btnConnect = new QPushButton("Connect");

    topLayout->addWidget(new QLabel("Port:"));
    topLayout->addWidget(m_portCombo, 1);
    topLayout->addWidget(new QLabel("Baud:"));
    topLayout->addWidget(m_baudCombo);
    topLayout->addWidget(m_btnRefresh);
    topLayout->addWidget(m_btnConnect);

    // Log
    m_logView = new QTextEdit();
    m_logView->setReadOnly(true);
    m_logView->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; font-family: Consolas;");

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

    // Connections
    connect(m_btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshPorts);
    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_btnSend, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(m_sendEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);
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
    m_portCombo->setEnabled(!connected);
    m_baudCombo->setEnabled(!connected);
    m_btnRefresh->setEnabled(!connected);
    m_btnSend->setEnabled(connected);
    m_sendEdit->setEnabled(connected);

    m_btnConnect->setText(connected ? "Disconnect" : "Connect");
}

void MainWindow::onConnectClicked()
{
    if (m_serial && m_serial->isOpen()) {
        m_serial->close();
        m_serial->deleteLater();
        m_serial = nullptr;
        setConnectedState(false);
        m_statusLabel->setText("Status: Disconnected");
        m_logView->append("<span style='color:#f85149;'>Disconnected</span>");
        return;
    }

    if (m_portCombo->currentData().isNull()) {
        QMessageBox::warning(this, "Warning", "No valid port selected.");
        return;
    }

    QString portName = m_portCombo->currentData().toString();
    qint32 baud = m_baudCombo->currentText().toInt();

    m_serial = new SerialCommunication(portName, baud, this);

    connect(m_serial, &SerialCommunication::dataReceived,
            this, &MainWindow::onDataReceived);
    connect(m_serial, &SerialCommunication::stateChanged,
            this, &MainWindow::onStateChanged);
    connect(m_serial, &SerialCommunication::errorOccurred,
            this, &MainWindow::onErrorOccurred);

    if (m_serial->open()) {
        setConnectedState(true);
        m_statusLabel->setText("Status: Connected to " + portName);
        m_logView->append("<span style='color:#3fb950;'>Connected to " + portName + "</span>");
    } else {
        m_serial->deleteLater();
        m_serial = nullptr;
    }
}

void MainWindow::onSendClicked()
{
    if (!m_serial || !m_serial->isOpen())
        return;

    QString text = m_sendEdit->text();
    if (text.isEmpty())
        return;

    QByteArray data = (text + "\r\n").toUtf8();
    m_serial->send(data);

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
    Q_UNUSED(state);
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_logView->append("<span style='color:#f85149;'>Error: " + error.toHtmlEscaped() + "</span>");
    setConnectedState(false);
    m_statusLabel->setText("Status: Error");
}

} // namespace edcc