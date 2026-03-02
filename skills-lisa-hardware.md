Here is a comprehensive hardware architecture and programming model guide for the Apple Lisa, based on the provided Lisa Hardware Manual. This guide is tailored to assist an LLM in reverse-engineering and understanding Lisa system software, OS-level code, and drivers.

### 1. Architecture Overview

The Apple Lisa is built around the Motorola 68000 CPU operating at 5 MHz. The system employs a custom Memory Management Unit (MMU) for address translation, interleaved video memory, and several discrete I/O subsystems.

#### 1.1 Physical Address Spaces

Logical addresses (24-bit, 16 MB) emitted by the CPU are translated by the MMU into one of three distinct physical spaces:

* **Main Memory:** Up to 2 MB of System RAM.
* **I/O Space:** Used to access peripheral controllers and status/control registers.
* **Special I/O Space:** Used to access the Boot ROM and MMU translation registers.

---

### 2. Memory Management Unit (MMU)

The MMU provides address translation and memory protection. It implements four independent **Contexts** (Context 0 is for the OS/Supervisor, 1-3 for User programs).

#### 2.1 Address Translation

A 24-bit logical address is split into:

* **Bits 17-23:** Logical Segment (128 segments total).
* **Bits 9-16:** Logical Page Displacement.
* **Bits 0-8:** Offset within the 512-byte page.

Each of the 128 logical segments has two 12-bit registers associated with it:

* **SOR (Segment Origin Register):** Contains the 12-bit physical page address where the segment begins.
* **SLR (Segment Limit Register):** Contains the segment size limit (in pages) in the lower 8 bits, and 4 access control bits in the upper 4 bits.

#### 2.2 MMU Access Control Bits (SLR High Nibble)

* `0010` - Main memory: Read only from stack
* `0101` - Main memory: Read only
* `0110` - Main memory: Read/Write to stack
* `0111` - Main memory: Read/Write
* `1001` - I/O space
* `1010` - Special I/O space
* `0000`, `0001`, `0011`, `0100`, `1000`, `1111` - Page invalid

#### 2.3 Context Switching

Context is controlled by setting/resetting `SEG1` and `SEG2` flip-flops via Special I/O addresses:

* **Set/Reset SEG1:** Read/Write `00E00AH` (Set), `00E008H` (Reset)
* **Set/Reset SEG2:** Read/Write `00E00EH` (Set), `00E00CH` (Reset)

---

### 3. System I/O Map

When the MMU determines an address resides in I/O space, physical addresses map as follows:

| Address Range | Function |
| --- | --- |
| `000000 - 001FFF` | Expansion slot #1 (Low Decode) |
| `002000 - 003FFF` | Expansion slot #1 (High Decode) |
| `004000 - 005FFF` | Expansion slot #2 (Low Decode) |
| `006000 - 007FFF` | Expansion slot #2 (High Decode) |
| `008000 - 009FFF` | Expansion slot #3 (Low Decode) |
| `00A000 - 00BFFF` | Expansion slot #3 (High Decode) |
| `FCC001 - FCC7FF` | Floppy Disk Control |
| `00D000 - 00D3FF` | Serial Ports Control |
| `00D400 - 00D7FF` | 9512 Floating Point Unit (Optional) |
| `00D800 - 00DBFF` | Parallel Port Control |
| `00DC00 - 00DFFF` | Keyboard/Mouse Control |
| `00E000 - 00E01E` | CPU Board Control |
| `00E8xx` | Video Address Latch |
| `00F0xx` | Memory Error Address Latch |
| `00F8xx` | System Status Register |

---

### 4. Processor Board Control & Status

#### 4.1 Control Latches

Accessed via Read/Write to set or reset internal hardware states:

* `00E000` (Reset) / `00E002` (Set): Memory Diagnostic DIAG1
* `00E004` (Reset) / `00E006` (Set): Memory Diagnostic DIAG2
* `00E008` (Reset) / `00E00A` (Set): Context Selection SEG1
* `00E00C` (Reset) / `00E00E` (Set): Context Selection SEG2
* `00E010` (Reset) / `00E012` (Set): SETUP Register (Map disable)
* `00E014` (Reset) / `00E016` (Set): Enable Soft Memory Error Detect
* `00E018` (Reset) / `00E01A` (Set): Enable Vertical Retrace Interrupt
* `00E01C` (Reset) / `00E01E` (Set): Enable Hard Memory Error Detect

#### 4.2 System Status Register (`00F800H` Read-Only)

* **Bit 0:** Soft Memory Error occurred.
* **Bit 1:** Hard Memory Error occurred.
* **Bit 2:** Vertical Retrace in progress.
* **Bit 3:** Bus Timeout (30-300 µs elapsed without AS/ response).
* **Bit 4:** Video Bit (Current pixel stream output).
* **Bit 5:** Horizontal Sync status.

---

### 5. Floppy Disk Controller (FDC)

Driven by a dedicated 6504 processor sharing 1KB of RAM (`FCC001 - FCC7FF`) with the 68000.

* **Command Byte:** Written to `FCC001`
* `81H`: Execute RWTS (Read/Write/Track/Sector) routine
* `83H`: Seek to side/track
* `85H`: Clear Interrupt Status
* `86H`: Set Interrupt Mask
* `87H`: Clear Interrupt Mask
* `89H`: Loop in ROM


* **Command Block Parameters:**
* `FCC003`: Command Code (`00`=Read, `01`=Write, `02`=Unclamp, `03`=Format, `04`=Verify, `07`=Read without checksum, `08`=Write without checksum generation)
* `FCC005`: Drive Select (`00`=Drive 2, `80H`=Drive 1)
* `FCC007`: Side Select (`0x`=Upper, `1x`=Lower)
* `FCC009`: Sector Number (0-22)
* `FCC00B`: Track Number (0-44)
* `FCC011`: Error Status


* **I/O Buffer:** `FCC501 - FCC7FF` (524 bytes).

---

### 6. Peripheral Interfaces (Serial, Parallel, Keyboard)

#### 6.1 Serial Ports (8530 SCC)

* **Channel A:** Data = `00D247`, Control = `00D243`
* **Channel B:** Data = `00D245`, Control = `00D241`

#### 6.2 Parallel Port (6522 VIA)

Mapped from `00D901` to `00D979`. Base registers:

* `00D901`: ORB/IRB
* `00D909`: ORA/IRA
* `00D911`: DDRB
* `00D919`: DDRA

#### 6.3 Keyboard, Mouse, and RTC (6522 VIA + COPS)

Mapped from `00DD81` to `00DD9F` (Base registers mirror the Parallel Port 6522).
The COPS slave processor is accessed via the 6522 Port A (`00DD83`).
**COPS Commands (Written to Port A):**

* `0000 0000`: Turn I/O port on
* `0000 0001`: Turn I/O port off
* `0000 0010`: Read Real-Time Clock data
* `0111 ennn`: Mouse setup (`e`=enable interrupts, `nnn`=interval in 4ms steps)

**Mouse Data:** Polled as 3 bytes indicating status, X-change (-127 to 127), and Y-change (-127 to 127).

---

### 7. Interrupt Hardware Structure

Interrupts vector through the standard 68000 exception table using fixed hardware priorities:

* **Level 7 (NMI):** Power fail, Hard memory error, Soft memory error, Keyboard reset.
* **Level 6:** Serial I/O ports.
* **Level 5, 4, 3:** Expansion slots 1, 2, 3 respectively.
* **Level 2:** Keyboard, Mouse, Real-Time Clock, Power-on.
* **Level 1:** Hard Disk (parallel port), Floppy Disk, Video Circuit (Vertical retrace).

### 8. Video Generation

* **Resolution:** 720 x 364 pixels (with 15 additional non-displayed retrace lines).
* **Memory:** Driven by a bit-map located in main System RAM (occupies exactly 32 KB).
* **Video Address Latch:** The upper 6 bits of the physical address pointing to the video page are written to `00E800H`.