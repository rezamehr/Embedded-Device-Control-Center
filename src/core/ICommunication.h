#pragma once

#include <QObject>
#include <QByteArray>
#include "Types.h"

namespace edcc {

class ICommunication : public QObject
{
    Q_OBJECT

public:
    explicit ICommunication(QObject *parent = nullptr) : QObject(parent) {}
    ~ICommunication() override = default;

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual qint64 send(const QByteArray &data) = 0;
    virtual QString name() const = 0;

signals:
    void dataReceived(const QByteArray &data);
    void stateChanged(edcc::ConnectionState state);
    void errorOccurred(const QString &errorString);
};

} // namespace edcc
