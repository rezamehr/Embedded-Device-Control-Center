#pragma once

#include "IMemoryTarget.h"
#include <QByteArray>
#include <QMap>

namespace edcc {

/**
 * @brief A simple in-memory implementation of IMemoryTarget for testing.
 *
 * This class simulates flash memory in RAM.
 * Useful for unit testing FirmwareUpdater without real hardware.
 */
class DummyMemoryTarget : public IMemoryTarget
{
    Q_OBJECT

public:
    explicit DummyMemoryTarget(QObject *parent = nullptr);
    ~DummyMemoryTarget() override = default;

    // IMemoryTarget interface
    MemoryTargetInfo info() const override;
    bool isReady() const override;

    bool erase(quint32 address, quint32 size) override;
    bool massErase() override;

    bool write(quint32 address, const QByteArray &data) override;
    QByteArray read(quint32 address, quint32 size) override;
    bool verify(quint32 address, const QByteArray &expected) override;

    bool jumpTo(quint32 address) override;

    // Test helpers
    void setReady(bool ready);
    void reset();                       ///< Clear all simulated memory
    QByteArray dump(quint32 address, quint32 size) const;

private:
    MemoryTargetInfo m_info;
    bool m_ready = true;

    // Simulated memory (address → data)
    QMap<quint32, quint8> m_memory;
};

} // namespace edcc
