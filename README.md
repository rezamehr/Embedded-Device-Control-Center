# Embedded-Device-Control-Center
Qt + CMake + Visual Studio/MSVC
# Embedded Device Control Center (EDCC)

**A professional desktop application for discovering, managing, monitoring, and updating embedded devices.**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-6.11+-green.svg)](https://www.qt.io)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)]()

---

## Overview

**Embedded Device Control Center (EDCC)** is an industrial-grade desktop tool designed for engineers working with microcontrollers (STM32, ESP32, and similar devices).

### Key Features (Roadmap)

| Version | Features |
|---------|----------|
| **v1.0** | Device Manager, Serial Monitor, TCP Monitor, Central Log Viewer |
| **v1.5** | Multi-threaded Communication, Packet Parser, JSON Configuration |
| **v2.0** | Firmware Updater (Bootloader support) |
| **v3.0** | Real-time Charts, Alarm System |
| **v4.0** | Plugin Architecture, MQTT, CAN |

---

## Architecture

EDCC follows a clean layered architecture:

- **Application Layer** – UI and user interaction
- **Core Layer** – Device management and business logic
- **Communication Layer** – Serial & TCP abstraction
- **Logging Layer** – Centralized logging system

Detailed architecture is available in [`docs/architecture.md`](docs/architecture.md).

---

## Getting Started

### Requirements

- Qt 6.11 or later
- CMake 3.21+
- C++17 compatible compiler (MSVC 2022 / GCC / Clang)

### Build Instructions

```bash
git clone https://github.com/rezamehr/Embedded-Device-Control-Center.git
cd Embedded-Device-Control-Center
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

## Project Status

**Current Version:** `1.0.0`

EDCC v1.0 provides a solid foundation for managing and monitoring embedded devices over Serial and TCP, with multi-device support, centralized logging, and persistent configuration.
و در جدول Roadmap می‌توانی بنویسی:
Markdown| Version | Features                                              | Status     |
|---------|-------------------------------------------------------|------------|
| **v1.0** | Device Manager, Serial, TCP, Logger, JSON Config     | ✅ Released |
| **v1.5** | Multi-threaded Communication, Packet Parser          | Planned    |
| **v2.0** | Firmware Updater                                      | Planned    |
| **v3.0** | Real-time Charts, Alarm System                        | Planned    |
| **v4.0** | Plugin Architecture, MQTT, CAN                        | Planned    |

License
This project is licensed under the MIT License - see the LICENSE file for details.

Author
Reza Mehrabani
Embedded Systems Engineer
