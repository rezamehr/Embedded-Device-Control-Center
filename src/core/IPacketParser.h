#pragma once

#include <QObject>
#include <QByteArray>

namespace edcc {

/**
 * @brief Interface for packet parsers.
 *
 * A parser receives a continuous stream of bytes and emits
 * complete packets when they are detected.
 */
class IPacketParser : public QObject
{
    Q_OBJECT

public:
    explicit IPacketParser(QObject *parent = nullptr) : QObject(parent) {}
    ~IPacketParser() override = default;

    /**
     * @brief Feed new raw data into the parser.
     */
    virtual void feed(const QByteArray &data) = 0;

    /**
     * @brief Reset internal state (e.g. after disconnect).
     */
    virtual void reset() = 0;

signals:
    /**
     * @brief Emitted when a complete and valid packet is found.
     * @param packet  The extracted payload (without header/checksum)
     */
    void packetReady(const QByteArray &packet);

    /**
     * @brief Emitted when a corrupted or invalid frame is detected.
     */
    void parseError(const QString &reason);
};

} // namespace edcc
