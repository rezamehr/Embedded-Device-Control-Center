# Changelog

All notable changes to **Embedded Device Control Center (EDCC)** are documented in this file.

The format is based on Keep a Changelog, and this project adheres to Semantic Versioning.

## [2.0.0] - 2026-08-15

### Added
- **Firmware Update layer** (`src/firmware/`)
  - `IMemoryTarget` abstraction for erase / write / read / verify / jump
  - `FirmwareUpdater` with asynchronous worker thread
  - `DummyMemoryTarget` for offline testing
  - `SerialBootloaderTarget` for real device updates over managed `Device`
- **Bootloader Protocol v1.0**
  - Frame: `START(0xAA) + LENGTH(1) + PAYLOAD + CHECKSUM(XOR payload)`
  - Commands: Identity, Read, Erase, Write, Jump, ACK, NACK
  - Spec: `docs/bootloader_protocol.md`
- **UI Firmware panel** in MainWindow
  - Browse `.bin`, start address, progress, start/abort
- **Unit tests**
  - `test_firmware` – updater + dummy target
  - `test_serialbootloader` – protocol target via Device + Mock
  - Updated `test_protocol` for payload-only checksum

### Changed
- `SimplePacketParser` checksum now matches protocol v1.0 (XOR of payload only)
- `MockCommunication` emits `stateChanged(Connected/Disconnected)` for realistic Device tests

### Notes
- Host update sequence: Identity → Erase → Write chunks → Verify → Jump
- Device must be connected and in bootloader mode before starting update
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
