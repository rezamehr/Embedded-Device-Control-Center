#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QStackedWidget>

#include "communication/SerialCommunication.h"
#include "communication/TcpCommunication.h"

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
    void onConnectClicked();
    void onSendClicked();
    void onDataReceived(const QByteArray &data);
    void onStateChanged(edcc::ConnectionState state);
    void onErrorOccurred(const QString &error);

private:
    void setupUi();
    void setConnectedState(bool connected);
    void clearCurrentConnection();

    // UI elements
    QComboBox *m_typeCombo;          // Serial / TCP
    QComboBox *m_portCombo;
    QComboBox *m_baudCombo;
    QLineEdit *m_hostEdit;
    QSpinBox  *m_portSpin;
    QPushButton *m_btnRefresh;
    QPushButton *m_btnConnect;
    QTextEdit *m_logView;
    QLineEdit *m_sendEdit;
    QPushButton *m_btnSend;
    QLabel *m_statusLabel;

    QWidget *m_serialControls;
    QWidget *m_tcpControls;

    // Current communication object
    ICommunication *m_comm = nullptr;
};

} // namespace edcc
