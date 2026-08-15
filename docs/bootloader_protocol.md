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






























FieldSizeDescriptionSTART1 byteAlways 0xAALENGTH1 byteLength of PAYLOAD only (0x00 … 0xFF)PAYLOADN bytesCommand byte + optional parameters/dataCHECKSUM1 byteXOR of all PAYLOAD bytes (not START, not LENGTH)
Checksum example
Payload = FF 50 04
text0xFF ⊕ 0x50 ⊕ 0x04 = 0xAB
Endianness
All multi-byte integers are little-endian.

2. Command Codes





















































NameCodeDirectionDescriptionCMD_IDENTITY0x01PC → MCUQuery device informationCMD_READ0x02PC → MCURead memoryCMD_ERASE0x03PC → MCUErase memory rangeCMD_WRITE0x04PC → MCUWrite memoryCMD_JUMP0x05PC → MCUJump to applicationCMD_ACK0xFFMCU → PCSuccess responseCMD_NACK0x1FMCU → PCError response

3. IDENTITY
Query flash parameters and bootloader version.
Request (PC → MCU)
textPAYLOAD:  01
FRAME:    AA 01 01 01






























ByteValueMeaning0AASTART101LENGTH = 1201CMD_IDENTITY301CHECKSUM
Success response (MCU → PC)
textPAYLOAD layout (12 bytes):
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

























FieldValueSTARTAALENGTH0C (12)PAYLOADFF 50 04 00 00 20 00 00 00 02 00 01CHECKSUM88
Error response
textAA 01 1F 1F

4. ERASE
Erase a memory range starting at address for size bytes.
Request (PC → MCU)
textPAYLOAD layout (9 bytes):
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
text03 ⊕ 00 ⊕ 40 ⊕ 00 ⊕ 08 ⊕ 00 ⊕ 00 ⊕ 01 ⊕ 00 = 0x4A
Success response
textAA 01 FF FF
Error response
textAA 01 1F 1F
Note: Sector erase on STM32H7 may take several seconds.
The host waits up to ~5 seconds for the response.
Send ACK only after erase has fully completed.

5. WRITE
Write a data chunk to flash/RAM.
Request (PC → MCU)
textPAYLOAD layout:
  [0]       CMD_WRITE = 0x04
  [1..4]    Address   = uint32 LE
  [5..N]    Data      = N-5 bytes

Example (4 data bytes at 0x08004000):
  PAYLOAD:  04 00 40 00 08 11 22 33 44
  LENGTH:   09
  FRAME:    AA 09 04 00 40 00 08 11 22 33 44 <CS>
Recommended max chunk size: 256 bytes (maxWriteChunk).
Success response
textAA 01 FF FF
Error response
textAA 01 1F 1F

6. READ
Read memory from the target.
Request (PC → MCU)
textPAYLOAD layout (7 bytes):
  [0]       CMD_READ = 0x02
  [1..4]    Address  = uint32 LE
  [5..6]    Size     = uint16 LE

Example (read 8 bytes from 0x08004000):
  PAYLOAD:  02 00 40 00 08 08 00
  FRAME:    AA 07 02 00 40 00 08 08 00 <CS>
Success response
textPAYLOAD:
  FF                ACK
  <data bytes>      exact number requested

Example (8 bytes):
  AA 09 FF 11 22 33 44 55 66 77 88 <CS>
Error response
textAA 01 1F 1F

7. JUMP
Jump to application entry point (usually after a successful update).
Request (PC → MCU)
textPAYLOAD layout (5 bytes):
  [0]       CMD_JUMP = 0x05
  [1..4]    Address  = uint32 LE

Example (jump to 0x08004000):
  PAYLOAD:  05 00 40 00 08
  FRAME:    AA 05 05 00 40 00 08 <CS>
Success response (optional)
textAA 01 FF FF
After JUMP the device leaves bootloader mode. The host should treat the
session as closed even if no ACK is received (device may reset immediately).

8. Quick Reference Table









































CommandExample requestSuccess responseError responseIDENTITYAA 01 01 01AA 0C FF 50 04 00 00 20 00 00 00 02 00 01 88AA 01 1F 1FERASEAA 09 03 00 40 00 08 00 00 01 00 4AAA 01 FF FFAA 01 1F 1FWRITEAA 09 04 00 40 00 08 11 22 33 44 CSAA 01 FF FFAA 01 1F 1FREADAA 07 02 00 40 00 08 08 00 CSAA … FF + data + CSAA 01 1F 1FJUMPAA 05 05 00 40 00 08 CSAA 01 FF FF (optional)AA 01 1F 1F

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
text1. Open connection (Serial / TCP)
2. IDENTITY          → read flash size / page size
3. ERASE             → erase range for the new image
4. WRITE (chunks)    → program firmware
5. READ / VERIFY     → optional integrity check
6. JUMP              → start application

11. Related EDCC Source Files

























FileRolesrc/firmware/protocol/BootloaderProtocol.hFrame builders & command IDssrc/firmware/SerialBootloaderTarget.*Host-side protocol implementationsrc/firmware/FirmwareUpdater.*High-level update enginesrc/communication/SimplePacketParser.*Shared frame parser

12. Version





























ItemValueProtocol version1.0Frame START0xAALength field1 byteChecksumXOR(payload)Byte orderLittle-endian
text
