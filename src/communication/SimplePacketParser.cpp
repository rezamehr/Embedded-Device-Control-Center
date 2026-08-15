#include "SimplePacketParser.h"
#include <QDebug>

namespace edcc {

SimplePacketParser::SimplePacketParser(QObject *parent)
    : IPacketParser(parent)
{
}

void SimplePacketParser::reset()
{
    m_state = State::WaitStart;
    m_expectedLength = 0;
    m_payload.clear();
    m_checksum = 0;
}

void SimplePacketParser::feed(const QByteArray &data)
{
    for (char c : data) {
        processByte(static_cast<quint8>(c));
    }
}

void SimplePacketParser::processByte(quint8 byte)
{
    switch (m_state) {

    case State::WaitStart:
        if (byte == START_BYTE) {
            m_state = State::WaitLength;
            m_expectedLength = 0;
            m_checksum = 0;
            m_payload.clear();
        }
        break;

    case State::WaitLength:
        // LENGTH = 1 byte (0..255)
        m_expectedLength = byte;
        m_checksum = 0;          //  payload => checksum
        m_payload.clear();
        m_payload.reserve(m_expectedLength);

        if (m_expectedLength == 0) {
            m_state = State::WaitChecksum;
        } else {
            m_state = State::WaitPayload;
        }
        break;

    case State::WaitPayload:
        m_payload.append(static_cast<char>(byte));
        m_checksum ^= byte;

        if (m_payload.size() >= static_cast<int>(m_expectedLength)) {
            m_state = State::WaitChecksum;
        }
        break;

    case State::WaitChecksum:
        qDebug() << "[Parser] m_checksum =" << m_checksum;
        qDebug() << "[Parser] byte =" << byte;
        if (byte == m_checksum) {
            emit packetReady(m_payload);
        } else {
            emit parseError(
                QStringLiteral("Checksum mismatch (got 0x%1, expected 0x%2)")
                    .arg(byte, 2, 16, QChar('0'))
                    .arg(m_checksum, 2, 16, QChar('0')));
        }

        // intial
        m_state = State::WaitStart;
        m_payload.clear();
        m_expectedLength = 0;
        m_checksum = 0;
        break;
    }
}


} // namespace edcc
