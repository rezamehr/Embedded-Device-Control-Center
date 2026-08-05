
#pragma once

#include "core/ICommunication.h"
#include <QSerialPort>

namespace edcc {

class SerialCommunication : public ICommunication
{
    Q_OBJECT

public:
    explicit SerialCommunication(const QString &portName,
                                 qint32 baudRate = 115200,
                                 QObject *parent = nullptr);
    ~SerialCommunication() override;

    bool open() override;
    void close() override;
    bool isOpen() const override;
    qint64 send(const QByteArray &data) override;
    QString name() const override;

    void setBaudRate(qint32 baudRate);
    qint32 baudRate() const;

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serial;
    QString m_portName;
    qint32 m_baudRate;
};

} // namespace edcc
