
# EDCC Architecture

## Overview

Embedded Device Control Center (EDCC) is designed with a clean, layered, and modular architecture to support long-term maintainability, testability, and extensibility.

The architecture follows these principles:

- Separation of Concerns
- Dependency Inversion (depend on abstractions)
- Single Responsibility
- Easy to extend with new communication protocols (future Plugin system)

---

## High-Level Layers

```text
┌─────────────────────────────────────────────┐
│              Application Layer              │
│         (MainWindow, Widgets, UI)           │
└─────────────────────┬───────────────────────┘
                      │
┌─────────────────────▼───────────────────────┐
│                 Core Layer                  │
│     (DeviceManager, Device, Services)       │
└─────────────────────┬───────────────────────┘
                      │
┌─────────────────────▼───────────────────────┐
│            Communication Layer              │
│      (Serial, TCP, Future: CAN, MQTT)       │
└─────────────────────┬───────────────────────┘
                      │
┌─────────────────────▼───────────────────────┐
│               Logging Layer                 │
│          (Logger, Log Storage)              │
└─────────────────────────────────────────────┘

Layer Responsibilities
1. Application Layer (src/app)

Contains all UI components
MainWindow and different views (Device Manager, Monitors, Log Viewer)
Only talks to the Core layer
Must not contain business logic

2. Core Layer (src/core)

Central brain of the application
Manages list of devices
Coordinates communication sessions
Owns high-level business rules
Defines main interfaces (IDevice, ICommunication, etc.)

3. Communication Layer (src/communication)

Implements actual transport protocols
SerialCommunication
TcpCommunication
Later: CanCommunication, MqttCommunication
All implement a common interface (ICommunication)

4. Logging Layer (src/logging)

Centralized logging system
Receives logs from all layers
Supports filtering, searching, and persistence (SQLite later)


Key Interfaces (v1.0)
ICommunication
C++class ICommunication {
public:
    virtual ~ICommunication() = default;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual qint64 send(const QByteArray &data) = 0;
    // signals: dataReceived, errorOccurred, stateChanged
};
IDevice
C++class IDevice {
public:
    virtual ~IDevice() = default;
    virtual QString name() const = 0;
    virtual QString id() const = 0;
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};

Design Decisions (v1.0)

UI is built with Qt Widgets
Communication runs in worker threads (from v1.5)
Configuration will be stored in JSON
Logging is centralized from day one
No Plugin system in v1.0 (will be introduced later)


Future Extensions

Plugin SDK (v4.0+)
Firmware Update Engine
Real-time Charts
Multi-device simultaneous sessions
Scripting support
