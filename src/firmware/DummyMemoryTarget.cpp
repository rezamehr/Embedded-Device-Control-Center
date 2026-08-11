#include "DummyMemoryTarget.h"
#include "logging/Logger.h"

namespace edcc {

DummyMemoryTarget::DummyMemoryTarget(QObject *parent)
    : IMemoryTarget(parent)
{
    // Default simulated flash: 512 KB starting at 0x08000000
    m_info.name = QStringLiteral("Dummy Internal Flash");
    m_info.startAddress = 0x08000000;
    m_info.size = 512 * 1024;          // 512 KB
    m_info.pageSize = 2048;            // 2 KB pages
    m_info.writeAlignment = 4;
    m_info.supportsDualBank = false;
    m_info.supportsReadWhileWrite = false;
    m_info.maxWriteChunk = 256;
}

MemoryTargetInfo DummyMemoryTarget::info() const
{
    return m_info;
}

bool DummyMemoryTarget::isReady() const
{
    return m_ready;
}

bool DummyMemoryTarget::erase(quint32 address, quint32 size)
{
    if (!m_ready) {
        emit errorOccurred(tr("Target is not ready"));
        return false;
    }

    if (address < m_info.startAddress ||
        (address + size) > (m_info.startAddress + m_info.size)) {
        emit errorOccurred(tr("Erase address out of range"));
        return false;
    }

    // Simulate erase by filling with 0xFF
    for (quint32 i = 0; i < size; ++i) {
        m_memory[address + i] = 0xFF;
    }

    Logger::instance().info(QStringLiteral("DummyMemoryTarget"),
                            tr("Erased %1 bytes at 0x%2")
                                .arg(size)
                                .arg(address, 8, 16, QChar('0')));

    return true;
}

bool DummyMemoryTarget::massErase()
{
    m_memory.clear();
    Logger::instance().info(QStringLiteral("DummyMemoryTarget"), tr("Mass erase completed"));
    return true;
}

bool DummyMemoryTarget::write(quint32 address, const QByteArray &data)
{
    if (!m_ready) {
        emit errorOccurred(tr("Target is not ready"));
        return false;
    }

    if (data.isEmpty()) {
        return true;
    }

    if (address < m_info.startAddress ||
        (address + data.size()) > (m_info.startAddress + m_info.size)) {
        emit errorOccurred(tr("Write address out of range"));
        return false;
    }

    for (int i = 0; i < data.size(); ++i) {
        m_memory[address + i] = static_cast<quint8>(data.at(i));
    }

    return true;
}

QByteArray DummyMemoryTarget::read(quint32 address, quint32 size)
{
    QByteArray result;
    result.resize(static_cast<int>(size));

    for (quint32 i = 0; i < size; ++i) {
        // Unwritten memory returns 0xFF (erased state)
        result[i] = static_cast<char>(m_memory.value(address + i, 0xFF));
    }

    return result;
}

bool DummyMemoryTarget::verify(quint32 address, const QByteArray &expected)
{
    const QByteArray actual = read(address, static_cast<quint32>(expected.size()));
    const bool ok = (actual == expected);

    if (!ok) {
        emit errorOccurred(tr("Verification mismatch at 0x%1")
                               .arg(address, 8, 16, QChar('0')));
    }

    return ok;
}

bool DummyMemoryTarget::jumpTo(quint32 address)
{
    Q_UNUSED(address);
    Logger::instance().info(QStringLiteral("DummyMemoryTarget"),
                            tr("Jump to 0x%1 simulated").arg(address, 8, 16, QChar('0')));
    return true;
}

// ==================== Test helpers ====================

void DummyMemoryTarget::setReady(bool ready)
{
    m_ready = ready;
}

void DummyMemoryTarget::reset()
{
    m_memory.clear();
}

QByteArray DummyMemoryTarget::dump(quint32 address, quint32 size) const
{
    QByteArray result;
    result.resize(static_cast<int>(size));

    for (quint32 i = 0; i < size; ++i) {
        result[i] = static_cast<char>(m_memory.value(address + i, 0xFF));
    }

    return result;
}

} // namespace edcc
