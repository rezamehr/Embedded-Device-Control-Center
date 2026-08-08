#include "CommunicationWorker.h"

namespace edcc {

CommunicationWorker::CommunicationWorker(ICommunication *communication,
                                         QObject *parent)
    : QObject(parent)
    , m_comm(communication)
{
    Q_ASSERT(m_comm);

    // Take ownership so the communication object lives in the same thread
    m_comm->setParent(this);

    connect(m_comm, &ICommunication::dataReceived,
            this, &CommunicationWorker::onCommDataReceived);
    connect(m_comm, &ICommunication::stateChanged,
            this, &CommunicationWorker::onCommStateChanged);
    connect(m_comm, &ICommunication::errorOccurred,
            this, &CommunicationWorker::onCommError);
}

CommunicationWorker::~CommunicationWorker()
{
    // Ensure the channel is closed when the worker is destroyed
    if (m_comm && m_comm->isOpen()) {
        m_comm->close();
    }
}

void CommunicationWorker::open()
{
    if (!m_comm)
        return;

    if (m_comm->open()) {
        emit opened();
    }
}

void CommunicationWorker::close()
{
    if (!m_comm)
        return;

    m_comm->close();
    emit closed();
}

void CommunicationWorker::send(const QByteArray &data)
{
    if (!m_comm || !m_comm->isOpen())
        return;

    const qint64 written = m_comm->send(data);
    if (written > 0) {
        emit bytesWritten(written);
    }
}

void CommunicationWorker::onCommDataReceived(const QByteArray &data)
{
    emit dataReceived(data);
}

void CommunicationWorker::onCommStateChanged(ConnectionState state)
{
    emit stateChanged(state);
}

void CommunicationWorker::onCommError(const QString &error)
{
    emit errorOccurred(error);
}

} // namespace edcc
