# Changelog

All notable changes to Embedded Device Control Center (EDCC) are documented in this file.

## [1.0.0] - 2026-08-08

### Added
- Core layered architecture (Application, Core, Communication, Logging)
- `ICommunication` and `IDevice` interfaces
- Serial communication support (`QSerialPort`)
- TCP communication support (`QTcpSocket`)
- Device and DeviceManager for multi-device handling
- Central Logger with log levels (Debug, Info, Warning, Error, Critical)
- JSON-based configuration for saving/loading devices
- MainWindow with:
  - Device list with connection status
  - Serial / TCP connection settings
  - Real-time log view with timestamps and colors
  - Send/Receive data
  - Clear Log and Save Log
  - TX / RX byte counters
- Basic professional project structure for GitHub

### Notes
- This is the first stable foundation release (v1.0).
- Multi-threading, packet parser, and firmware update are planned for later versions.
