#include "Device.h"
#include "communication/CommunicationWorker.h"
#include "logging/Logger.h"

namespace edcc {

Device::Device(const DeviceInfo &info,
               ICommunication *communication,
               QObject *parent)
    : IDevice(parent)
    , m_info(info)
{
    setupWorker(communication);
}

Device::~Device()
{
    // Clean shutdown of the worker thread
    if (m_thread) {
        if (m_worker) {
            // Request close from the worker thread
            QMetaObject::invokeMethod(m_worker, "close", Qt::QueuedConnection);
        }

        m_thread->quit();
        if (!m_thread->wait(3000)) {
            Logger::instance().warning("Worker thread did not finish in time, terminating", m_info.id);
            m_thread->terminate();
            m_thread->wait(1000);
        }
    }
}

void Device::setupWorker(ICommunication *communication)
{
    m_thread = new QThread(this);
    m_worker = new CommunicationWorker(communication); // no parent – we move it

    // Move worker to its dedicated thread
    m_worker->moveToThread(m_thread);

    // Clean up worker when thread finishes
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    // Forward signals from worker to Device (and then to outside)
    connect(m_worker, &CommunicationWorker::stateChanged,
            this, &Device::onWorkerStateChanged);
    connect(m_worker, &CommunicationWorker::dataReceived,
            this, &Device::onWorkerDataReceived);
    connect(m_worker, &CommunicationWorker::errorOccurred,
            this, &Device::onWorkerError);
    connect(m_worker, &CommunicationWorker::bytesWritten,
            this, &Device::onWorkerBytesWritten);

    m_thread->start();
}

QString Device::id() const
{
    return m_info.id;
}

QString Device::name() const
{
    return m_info.name;
}

QString Device::description() const
{
    return m_info.description;
}

bool Device::connectToDevice()
{
    if (!m_worker)
        return false;

    // Call open() in the worker thread
    QMetaObject::invokeMethod(m_worker, "open", Qt::QueuedConnection);
    return true; // actual result comes asynchronously via stateChanged
}

void Device::disconnectFromDevice()
{
    if (!m_worker)
        return;

    QMetaObject::invokeMethod(m_worker, "close", Qt::QueuedConnection);
}

bool Device::isConnected() const
{
    return m_state == ConnectionState::Connected;
}

ConnectionState Device::state() const
{
    return m_state;
}

qint64 Device::send(const QByteArray &data)
{
    if (!m_worker || m_state != ConnectionState::Connected)
        return -1;

    // Fire-and-forget – actual write happens in worker thread
    QMetaObject::invokeMethod(m_worker, "send",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
    return data.size(); // optimistic
}

DeviceInfo Device::info() const
{
    return m_info;
}

void Device::onWorkerStateChanged(ConnectionState state)
{
    m_state = state;
    emit stateChanged(state);
}

void Device::onWorkerDataReceived(const QByteArray &data)
{
    emit dataReceived(data);
}

void Device::onWorkerError(const QString &error)
{
    emit errorOccurred(error);
}

void Device::onWorkerBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
    // Can be used later for accurate TX statistics
}

} // namespace edcc
