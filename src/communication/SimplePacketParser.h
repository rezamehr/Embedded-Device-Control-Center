#pragma once

#include "core/IPacketParser.h"

namespace edcc {

/**
 * @brief Simple frame parser.
 *
 * Frame format:
 *   START (1 byte) + LENGTH (1 byte) + PAYLOAD (N bytes) + CHECKSUM (1 byte)
 *
 * Checksum = XOR of all LENGTH + PAYLOAD bytes.
 */
class SimplePacketParser : public IPacketParser
{
    Q_OBJECT

public:
    static constexpr quint8 START_BYTE = 0xAA;

    explicit SimplePacketParser(QObject *parent = nullptr);

    void feed(const QByteArray &data) override;

public slots:
    void reset() override;
private:
    enum class State {
        WaitStart,
        WaitLength,
        WaitPayload,
        WaitChecksum
    };

    State m_state = State::WaitStart;
    quint8 m_expectedLength = 0;
    QByteArray m_payload;
    quint8 m_checksum = 0;

    void processByte(quint8 byte);
};

} // namespace edcc
