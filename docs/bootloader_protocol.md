# EDCC Bootloader Protocol v1.0

This document defines the serial/TCP bootloader protocol used by
**Embedded Device Control Center (EDCC)** to update firmware on target devices
(STM32 and similar MCUs).

Both sides (PC host and MCU bootloader) **must** use the same frame format
and command set described below.

---

## 1. Frame Format

Every message (request or response) uses this frame:

```text
+--------+--------+------------------+----------+
| START  | LENGTH |     PAYLOAD      | CHECKSUM |
| 1 byte | 1 byte |    N bytes       | 1 byte   |
+--------+--------+------------------+----------+

Fiel        dSize        Description
START1      byteAlways   0xAA
LENGTH1     byte         Length of PAYLOAD only (0x00 … 0xFF)
PAYLOADN    bytes        Command byte + optional parameters/data
CHECKSUM1   byte         XOR of all PAYLOAD bytes (not START, not LENGTH)

#Checksum example
Payload = FF 50 04
0xFF ⊕ 0x50 ⊕ 0x04 = 0xAB

##2. Command Codes

Name,            Code,    Direction,        Description
CMD_IDENTITY,    0x01,    PC → MCU,         Query device information
CMD_READ,        0x02,    PC → MCU,         Read memory
CMD_ERASE,       0x03,    PC → MCU,         Erase memory range
CMD_WRITE,       0x04,    PC → MCU,         Write memory
CMD_JUMP,        0x05,    PC → MCU,         Jump to application
CMD_ACK,         0xFF,    MCU → PC,         Success response
CMD_NACK,        0x1F,    MCU → PC,         Error response

3. IDENTITY
Query flash parameters and bootloader version.
Request (PC → MCU)

PAYLOAD:  01
FRAME:    AA 01 01 01

Byte,    Value,    Meaning
0,        AA,      START
1,        01,      LENGTH = 1
2,        01,      CMD_IDENTITY
3,        01,      CHECKSUM

Success response (MCU → PC)
PAYLOAD layout (12 bytes):
  [0]       ACK          = 0xFF
  [1..2]    Device ID    = uint16 LE
  [3..6]    Flash size   = uint32 LE (bytes)
  [7..10]   Page/sector  = uint32 LE (bytes)
  [11]      Version      = uint8

Example:
  Device ID  = 0x0450
  Flash size = 0x00200000 (2 MB)
  Page size  = 0x00020000 (128 KB)
  Version    = 1

PAYLOAD:
  FF 50 04 00 00 20 00 00 00 02 00 01

FRAME:
  AA 0C FF 50 04 00 00 20 00 00 00 02 00 01 88

Field,    Value
START,     AA
LENGTH,    0C (12)
PAYLOAD,   FF 50 04 00 00 20 00 00 00 02 00 01
CHECKSUM,  88

Error response
AA 01 1F 1F

4. ERASE
Erase a memory range starting at address for size bytes.
Request (PC → MCU)
PAYLOAD layout (9 bytes):
  [0]       CMD_ERASE = 0x03
  [1..4]    Address   = uint32 LE
  [5..8]    Size      = uint32 LE

Example:
  Address = 0x08004000
  Size    = 0x00010000 (64 KB)

PAYLOAD:
  03 00 40 00 08 00 00 01 00

FRAME:
  AA 09 03 00 40 00 08 00 00 01 00 4A
Checksum:
03 ⊕ 00 ⊕ 40 ⊕ 00 ⊕ 08 ⊕ 00 ⊕ 00 ⊕ 01 ⊕ 00 = 0x4A
Success response
AA 01 FF FF

Error response
AA 01 1F 1F

Note: Sector erase on STM32H7 may take several seconds.
The host waits up to ~5 seconds for the response.
Send ACK only after erase has fully completed.

5. WRITE
Write a data chunk to flash/RAM.
Request (PC → MCU)
PAYLOAD layout:
  [0]       CMD_WRITE = 0x04
  [1..4]    Address   = uint32 LE
  [5..N]    Data      = N-5 bytes

Example (4 data bytes at 0x08004000):
  PAYLOAD:  04 00 40 00 08 11 22 33 44
  LENGTH:   09
  FRAME:    AA 09 04 00 40 00 08 11 22 33 44 <CS>
Recommended max chunk size: 256 bytes (maxWriteChunk).
Success response
AA 01 FF FF
Error response
AA 01 1F 1F

6. READ
Read memory from the target.
Request (PC → MCU)

PAYLOAD layout (7 bytes):
  [0]       CMD_READ = 0x02
  [1..4]    Address  = uint32 LE
  [5..6]    Size     = uint16 LE

Example (read 8 bytes from 0x08004000):
  PAYLOAD:  02 00 40 00 08 08 00
  FRAME:    AA 07 02 00 40 00 08 08 00 <CS>

Success response
PAYLOAD:
  FF                ACK
  <data bytes>      exact number requested

Example (8 bytes):
  AA 09 FF 11 22 33 44 55 66 77 88 <CS>

Error response
AA 01 1F 1F

7. JUMP
Jump to application entry point (usually after a successful update).
Request (PC → MCU)

PAYLOAD layout (5 bytes):
  [0]       CMD_JUMP = 0x05
  [1..4]    Address  = uint32 LE

Example (jump to 0x08004000):
  PAYLOAD:  05 00 40 00 08
  FRAME:    AA 05 05 00 40 00 08 <CS>

Success response (optional)
AA 01 FF FF

After JUMP the device leaves bootloader mode. The host should treat the
session as closed even if no ACK is received (device may reset immediately).

8. Quick Reference Table
Command,      Example request,      Success response,      Error response
IDENTITY,      AA 01 01 01,         AA 0C FF 50 04 00      AA 01 1F 1F
                                    00 20 00 00 00 02
                                    00 01 88,

ERASE,         AA 09 03 00 40 00    AA 01 FF FF,           AA 01 1F 1F
               08 00 00 01 00 4A,

WRITE,         AA 09 04 00 40 00    AA 01 FF FF,            AA 01 1F 1F
               08 11 22 33 44 CS,

READ,          AA 07 02 00 40 00    AA … FF + data + CS,    AA 01 1F 1F
               08 08 00 CS,

JUMP,          AA 05 05 00 40 00    AA 01 FF FF (optional), AA 01 1F 1F
               08 CS,



9. MCU Implementation Rules

Accept only frames starting with 0xAA.
Treat LENGTH as one byte.
Compute checksum as XOR of PAYLOAD only.
On checksum error: ignore the frame or reply with NACK.
Always wrap responses in the same frame format.
For ERASE: reply only after the erase operation finishes.
Align WRITE addresses to the device programming granularity
(e.g. 32 bytes on STM32H7).
Keep bootloader responses small and deterministic for easy debugging.


10. Typical Update Sequence (Host)
1. Open connection (Serial / TCP)
2. IDENTITY          → read flash size / page size
3. ERASE             → erase range for the new image
4. WRITE (chunks)    → program firmware
5. READ / VERIFY     → optional integrity check
6. JUMP              → start application


11. Related EDCC Source Files

File,                                            Role
src/firmware/protocol/BootloaderProtocol.h,      Frame builders & command IDs
src/firmware/SerialBootloaderTarget.*,           Host-side protocol implementation
src/firmware/FirmwareUpdater.*,                  High-level update engine
src/communication/SimplePacketParser.*,          Shared frame parser

12. Version
Item,                Value
Protocol version,    1.0
Frame START,         0xAA
Length field,        1 byte
Checksum,            XOR(payload)
Byte order,          Little-endian

