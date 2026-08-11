#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include "core/Types.h"

namespace edcc {

/**
 * @brief Information about a memory target (Flash, External Flash, etc.)
 */
struct MemoryTargetInfo
{
    QString name;                   ///< Human readable name (e.g. "Internal Flash")
    quint32 startAddress = 0;       ///< Base address of the memory
    quint32 size = 0;               ///< Total size in bytes
    quint32 pageSize = 0;           ///< Smallest erasable unit (page or sector)
    quint32 writeAlignment = 4;     ///< Minimum write alignment (usually 4 or 8)
    bool    supportsDualBank = false;
    bool    supportsReadWhileWrite = false;
    quint32 maxWriteChunk = 256;    ///< Recommended max chunk size for writing
};

/**
 * @brief Abstract interface for any programmable memory target.
 *
 * This interface is designed to be independent of the underlying
 * flash technology (page erase, sector erase, dual-bank, external flash...).
 * Concrete implementations will handle the specific hardware details.
 */
class IMemoryTarget : public QObject
{
    Q_OBJECT

public:
    explicit IMemoryTarget(QObject *parent = nullptr) : QObject(parent) {}
    ~IMemoryTarget() override = default;

    // === Information ===
    virtual MemoryTargetInfo info() const = 0;
    virtual bool isReady() const = 0;

    // === Erase ===
    /**
     * @brief Erase a range of memory.
     * @param address  Start address (must be aligned to page/sector)
     * @param size     Number of bytes to erase
     * @return true if erase command was accepted
     */
    virtual bool erase(quint32 address, quint32 size) = 0;

    /**
     * @brief Erase the entire memory target.
     */
    virtual bool massErase() = 0;

    // === Write ===
    /**
     * @brief Write data to the memory target.
     * @param address  Destination address
     * @param data     Data to write
     * @return true if write was successful
     */
    virtual bool write(quint32 address, const QByteArray &data) = 0;

    // === Read ===
    /**
     * @brief Read data from the memory target.
     * @param address  Source address
     * @param size     Number of bytes to read
     * @return The read data (empty on failure)
     */
    virtual QByteArray read(quint32 address, quint32 size) = 0;

    // === Verify ===
    /**
     * @brief Verify a region against given data.
     * @return true if the memory matches the provided data
     */
    virtual bool verify(quint32 address, const QByteArray &expected) = 0;

    // === Execute / Jump ===
    /**
     * @brief Jump to the given address (usually after a successful update).
     */
    virtual bool jumpTo(quint32 address) = 0;

signals:
    void progressChanged(int percent);               ///< 0..100
    void operationFinished(bool success, const QString &message);
    void errorOccurred(const QString &errorString);
};

} // namespace edcc
