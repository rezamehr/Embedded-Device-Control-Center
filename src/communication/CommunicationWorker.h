#pragma once

#include <QObject>
#include <QByteArray>
#include "core/ICommunication.h"
#include "core/Types.h"

namespace edcc {

/**
 * @brief Runs an ICommunication instance inside a worker thread.
 *
 * This class is moved to a QThread. All heavy I/O happens there.
 * Communication with the outside world is done only via signals/slots
 * (QueuedConnection), so it is thread-safe for the UI.
 */
class CommunicationWorker : public QObject
{
    Q_OBJECT

public:
    /**
     * @param communication  Ownership is taken by this worker.
     *                       Must not be used from other threads after this.
     */
    explicit CommunicationWorker(ICommunication *communication,
                                 QObject *parent = nullptr);
    ~CommunicationWorker() override;

public slots:
    /** Open the underlying communication channel (called in worker thread). */
    void open();

    /** Close the underlying communication channel. */
    void close();

    /** Send data through the channel. */
    void send(const QByteArray &data);

signals:
    void opened();
    void closed();
    void dataReceived(const QByteArray &data);
    void stateChanged(edcc::ConnectionState state);
    void errorOccurred(const QString &errorString);
    void bytesWritten(qint64 bytes);

private slots:
    void onCommDataReceived(const QByteArray &data);
    void onCommStateChanged(edcc::ConnectionState state);
    void onCommError(const QString &error);

private:
    ICommunication *m_comm = nullptr;   // owned
};

} // namespace edcc
