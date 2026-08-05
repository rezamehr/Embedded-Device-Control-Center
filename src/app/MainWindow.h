#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include "communication/SerialCommunication.h"

namespace edcc {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onRefreshPorts();
    void onConnectClicked();
    void onSendClicked();
    void onDataReceived(const QByteArray &data);
    void onStateChanged(edcc::ConnectionState state);
    void onErrorOccurred(const QString &error);

private:
    void setupUi();
    void setConnectedState(bool connected);

    QComboBox *m_portCombo;
    QComboBox *m_baudCombo;
    QPushButton *m_btnRefresh;
    QPushButton *m_btnConnect;
    QTextEdit *m_logView;
    QLineEdit *m_sendEdit;
    QPushButton *m_btnSend;
    QLabel *m_statusLabel;

    SerialCommunication *m_serial = nullptr;
};

} // namespace edcc