# Changelog

All notable changes to **Embedded Device Control Center (EDCC)** are documented in this file.

The format is based on Keep a Changelog, and this project adheres to Semantic Versioning.

---

## [1.5.0] - 2026-08-09

### Added
- Multi-threaded communication using Worker Object + `QThread`
- `CommunicationWorker` to keep Serial/TCP I/O off the UI thread
- Packet parser infrastructure (`IPacketParser`)
- `SimplePacketParser` with frame format:
  - `START (0xAA) + LENGTH + PAYLOAD + CHECKSUM`
- UI option: **Show raw data** (hex/raw stream vs parsed packets)
- Unit tests with Qt Test:
  - `test_protocol`
  - `test_jsonconfig`
- Safer device load/save flow for JSON configuration

### Fixed
- JSON `baudRate` truncation bug (`115200` no longer corrupted)
- Application shutdown crash related to device/thread cleanup
- More reliable device restore order on startup (signals connected before load)

### Improved
- Clearer multi-device UX (Add Device vs Connect/Disconnect)
- Device list status display
- Central logger integration across modules

---

## [1.0.0] - 2026-08-08

### Added
- Core layered architecture (Application, Core, Communication, Logging)
- Interfaces: `ICommunication`, `IDevice`
- Serial communication support
- TCP communication support
- `Device` and `DeviceManager` for multi-device handling
- Central Logger with levels (Debug, Info, Warning, Error, Critical)
- JSON-based device configuration (save/load)
- Main monitoring UI:
  - device list
  - connect/disconnect
  - send/receive
  - log view with timestamps
  - clear/save log
  - TX/RX counters

---

## [0.1.0-dev] - 2026-08-05

### Added
- Initial repository structure
- Project skeleton with CMake + Qt 6
- Architecture draft and foundational headers
