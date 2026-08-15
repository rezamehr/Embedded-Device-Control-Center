#pragma once

#include "core/ICommunication.h"
#include "core/Types.h"

#include <QByteArray>

namespace edcc {

/**
* @brief In-memory mock of ICommunication for unit tests.
 *
 * Call setNextResponse() before an operation that expects a reply.
 * The response is injected synchronously inside send().
 */
class MockCommunication : public ICommunication
{
    Q_OBJECT

public:
    explicit MockCommunication(QObject *parent = nullptr)
        : ICommunication(parent)
    {
    }

    bool open() override
    {
        if (m_open)
            return true;

        m_open = true;
        emit stateChanged(ConnectionState::Connected);  // ← importan
        return true;
    }

    void close() override
    {
        if (!m_open)
            return;

        m_open = false;
        emit stateChanged(ConnectionState::Disconnected); // ← importan
    }

    bool isOpen() const override
    {
        return m_open;
    }

    qint64 send(const QByteArray &data) override
    {
        m_lastSent = data;

        if (!m_nextResponse.isEmpty()) {
            const QByteArray response = m_nextResponse;
            m_nextResponse.clear();
            emit dataReceived(response);
        }

        return data.size();
    }

    QString name() const override
    {
        return QStringLiteral("Mock");
    }

    void setNextResponse(const QByteArray &response)
    {
        m_nextResponse = response;
    }

    QByteArray lastSent() const
    {
        return m_lastSent;
    }

private:
    bool m_open = false;
    QByteArray m_lastSent;
    QByteArray m_nextResponse;
};

} // namespace edcc
