# Skills: Interacting with LisaEm as an AI Agent

This document describes all knowledge and procedures required for an AI agent to control,
observe, and debug the LisaEm Apple Lisa emulator. It covers the socket control interface,
MCP bridge, boot sequencing, input injection, memory inspection, screenshot capture,
and Xenix-specific debugging workflows.

---

## 1. Environment and Paths

| Item | Value |
|------|-------|
| **Binary** | `bin/LisaEm.app/Contents/MacOS/lisaem-arm64-26.03-wx3.2.9` |
| **Launch working dir** | `/Users/peter/GitHub/lisaem/bin/LisaEm.app/Contents/MacOS/` |
| **TMPDIR** | `bin/LisaEm.app/Contents/MacOS/.tmp/` |
| **Socket** | `127.0.0.1:1983` (TCP, line-oriented) |
| **Console log** | `.tmp/lisaem-xenix-console.txt` |
| **MCP dump file** | `.tmp/lisaem-mcp-dump.bin` |
| **Screenshot file** | `.tmp/lisaem-screenshot.png` |
| **MCP state file** | `.tmp/lisaem-mcp-state.txt` |
| **Real config file** | `~/Library/Preferences/lisaem.conf` |

---

## 2. Launching the Emulator

```bash
cd /Users/peter/GitHub/lisaem/bin/LisaEm.app/Contents/MacOS
LISAEM_TMPDIR="$(pwd)/.tmp" nohup ./lisaem-arm64-26.03-wx3.2.9 >/dev/null 2>&1 &
```

- Always `cd` to the binary directory first — the emulator expects to find `.tmp/` relative to cwd.
- `LISAEM_TMPDIR` must be set so socket commands (`dump`, `screenshot`) write to the right place.
- The emulator may **auto-power-on** at startup if the config says so. Check `power=` in the first
  status response to know whether it started running already.
- Wait ~2 seconds after launch before using the socket.

Check the emulator is ready:
```bash
echo "status" | nc -w 2 127.0.0.1 1983
```

If `nc` returns nothing, the emulator is not yet listening. Retry after 1 second.

---

## 3. Socket Control Interface

The emulator listens on **TCP port 1983** for text commands (one per connection).
Send one line, read the response (ends when `mouse=` line is received), close.

### 3.1 Response Format

Every command returns a key=value block followed by a blank line:

```
reason=command
message=<human-readable result>
running=<1=running, 0=idle>
power=<1=powered on, 0=off>
pc=<8 hex digits, current 68000 PC>
clock=<16 hex digits, CPU cycle counter>
mouse=<x>,<y>
vram=<8 hex digits, physical VRAM start address>
copsqueuelen=<n>
via1ifr=<2 hex digits>
via1ier=<2 hex digits>
```

Relevant fields for debugging:
- `pc` — current 68000 program counter (physical address)
- `power` — whether the Lisa is powered on
- `running` — whether the emulation loop is active
- `vram` — physical base address of video RAM (needed for screenshot)
- `copsqueuelen` — number of bytes queued in the COPS keyboard queue
- `via1ifr` / `via1ier` — VIA interrupt flag/enable registers

### 3.2 Full Command Reference

| Command | Effect |
|---------|--------|
| `status` | Return current emulator state |
| `help` | Return list of commands |
| `power` | Toggle power button (on→off or off→on) |
| `shutdown` | Guest-managed shutdown (sends power key to running Lisa) |
| `quit` | Graceful quit: power off then exit |
| `force-shutdown` | Immediately halts emulation without guest shutdown |
| `pause` | Pause the emulation loop |
| `run` | Resume the emulation loop |
| `focus` | Give keyboard focus to the Lisa display window |
| `wait <n>` | Add ~n×200ms extra delay before next character in text queue |
| `apple1` | Send Apple+1 key chord (boot from floppy 1) |
| `apple2` | Send Apple+2 key chord (boot from floppy 2) |
| `apple3` | Send Apple+3 key chord (boot from ProFile hard disk) |
| `return` | Queue a carriage return into the text buffer |
| `text <ascii>` | Queue ASCII text + implicit CR into the COPS typing buffer |
| `keycode <n>` | Press and release key n (decimal or hex with 0x prefix) |
| `keycode raw <k> <state>` | Send raw COPS keycode byte: k=key num, state=0x80 (down) or 0x00 (up) |
| `move <x> <y>` | Move emulated mouse to pixel coordinates (0..719, 0..363) |
| `click <x> <y>` | Click emulated mouse at pixel coordinates |
| `dump <addr_hex> <len_hex>` | Dump physical RAM to `.tmp/lisaem-mcp-dump.bin`; returns "dumped N bytes" |
| `find <hex bytes...>` | Search physical RAM for byte pattern; returns "found @ XXXXXXXX" or "not found" |
| `screenshot` | Write current display to `.tmp/lisaem-screenshot.png`; returns path |
| `serial_a <hex bytes...>` | Inject bytes directly into Z8530 serial port A receive buffer |

### 3.3 Sending Commands with nc

```bash
# Single command:
echo "status" | nc -w 2 127.0.0.1 1983

# Command with argument:
echo "dump 0 400" | nc -w 2 127.0.0.1 1983

# Capture binary dump:
echo "dump 2000 800" | nc -w 2 127.0.0.1 1983
# Then read .tmp/lisaem-mcp-dump.bin
```

**Important:** Send **one command per nc invocation**. Piping multiple commands in sequence
(e.g. `echo "cmd1"; echo "cmd2"`) to the same nc call may not work; use separate Bash calls.

---

## 4. MCP Bridge (Python Layer)

`lisaem-mcp-bridge.py` wraps the socket interface as a JSON-RPC MCP server.
When Claude Code is configured to use it (via `.mcp.json`), the following tools are available:

| MCP Tool | Maps to | Notes |
|----------|---------|-------|
| `lisa_status` | `status` | Returns raw key=value text |
| `lisa_power` | `power` | Toggle power |
| `lisa_run_state` | `pause` / `run` | action field: "pause" or "run" |
| `lisa_input_text` | `text <t>` | Queued typing, CR appended automatically |
| `lisa_shortcut` | Various | Sends named shortcut by name |
| `lisa_input_mouse` | `move`/`click` | action + x,y |
| `lisa_memory_dump` | `dump` | addr_hex + len_hex; returns hex dump text + binary in dump file |
| `lisa_memory_find` | `find` | pattern_hex bytes |
| `lisa_xenix_console_tail` | File read | Reads last N bytes of lisaem-xenix-console.txt |
| `lisa_debug_command` | Raw socket | Sends any socket command string verbatim |
| `lisa_capture_trap_snapshot` | Combined | status + MCP state + console tail for crash analysis |
| `lisa_screenshot` | VRAM read | Reads VRAM via dump + status, synthesizes 1-bit PNG |

The `lisa_screenshot` MCP tool reads VRAM directly (more reliable than the socket `screenshot`
command which uses wxImage). It:
1. Calls `status` to get `vram=` address
2. Calls `dump <vram> 7FF8` (90 bytes/row × 364 rows = 32760 bytes)
3. Inverts bits (VRAM 1=black → PNG 1=white) and wraps in a PNG
4. Returns an image content block

---

## 5. Boot Sequence and Timing

### 5.1 Normal Xenix Boot (ProFile)

```
1. Launch emulator
2. Wait ~2s for socket to be ready
3. echo "power" | nc -w 3 127.0.0.1 1983        # power on
4. sleep 7                                         # wait for ROM boot menu
5. echo "apple3" | nc -w 3 127.0.0.1 1983        # select ProFile boot
   # PC should jump to 0x00022d4c (bootloader entry)
6. sleep 1
7. echo "keycode raw 48 80" | nc -w 3 127.0.0.1 1983  # Return down
8. echo "keycode raw 48 00" | nc -w 3 127.0.0.1 1983  # Return up
   # PC moves to ~0x000235a8 (kernel loading begins)
```

### 5.2 Timing Rules

- **7 seconds minimum** after `power` before sending `apple3`. ROM needs time to initialize.
- If `apple3` is sent too early, the ROM consumes the keycodes but doesn't advance.
  Symptom: `copsqueuelen` spikes to 4-5 but PC stays at `0xfe2dce`.
- After `apple3` succeeds, PC jumps immediately to `0x22d4c`.
- After Return, PC moves to `0x235a8` and ProFile block loading begins.
- The kernel loads ~57 ProFile blocks (visible via `[xenix-pf-*]` log entries).
- The HLE trap at PC=`0x241ac` fires when bootloader hands off to the kernel.

### 5.3 Checking Whether apple3 Worked

```bash
echo "status" | nc -w 2 127.0.0.1 1983 | grep "^pc="
```
- `pc=fe2dce` or `pc=fe2dc6` → still in ROM menu, apple3 not accepted yet
- `pc=0002xxxx` → bootloader running (success)

### 5.4 Force Restart

If the emulator crashes or gets stuck:
```bash
echo "force-shutdown" | nc -w 2 127.0.0.1 1983  # stop the running Lisa
sleep 1
echo "power" | nc -w 3 127.0.0.1 1983           # power on fresh
# then wait 7s and repeat boot sequence
```

Note: `force-shutdown` does NOT quit the emulator process. It stops the emulated Lisa
so a fresh boot can begin.

---

## 6. Keyboard Input

### 6.1 COPS Key Codes (Lisa Keyboard)

The Lisa uses a COPS slave processor for keyboard/mouse. Keycodes are 8-bit values:
- Bit 7 (`0x80`) = key-down event; bit 7 = 0 means key-up
- Bits 0-6 = physical key number

| Key | Keycode (7-bit) | Down byte | Up byte |
|-----|-----------------|-----------|---------|
| Return | 0x48 | `48 80` | `48 00` |
| Apple/Command | 0x7F | `7f 80` | `7f 00` |
| Space | 0x63 | `63 80` | `63 00` |
| Escape | 0x77 | `77 80` | `77 00` |
| Tab | 0x4C | `4c 80` | `4c 00` |
| 1 | 0x25 | `25 80` | `25 00` |
| 2 | 0x27 | `27 80` | `27 00` |
| 3 | 0x29 | `29 80` | `29 00` |

**COPS keycodes in socket protocol:** `keycode raw <k_hex> <state_hex>`
- state = `80` for key-down, `00` for key-up
- Always send matching down+up pairs to avoid stuck keys

### 6.2 Named Shortcuts (built-in socket commands)

- `apple1` — Apple+1 (boot floppy 1)
- `apple2` — Apple+2 (boot floppy 2 / insert Xenix floppy)
- `apple3` — Apple+3 (boot ProFile)
- `return` — Carriage return character

These use `send_cops_keycode()` directly, bypassing the `apple_combo_key()` queue-full guard.

### 6.3 Text Input

```bash
echo "text hello" | nc -w 2 127.0.0.1 1983
```
- Queues characters into `mcp_type_buf` (256-byte ring buffer)
- Appended CR is automatic if not already in string
- Delivery is ~200ms/character (throttled by emulation timer)
- `wait <n>` can be used to add extra delay between characters

---

## 7. Memory Inspection

### 7.1 dump Command

```bash
echo "dump <addr_hex> <len_hex>" | nc -w 2 127.0.0.1 1983
```
- Writes raw bytes to `.tmp/lisaem-mcp-dump.bin`
- Addresses are physical RAM addresses (0 to maxlisaram-1)
- For 2MB config: maxlisaram = 0x200000

```bash
# Read the dump as hex:
xxd .tmp/lisaem-mcp-dump.bin | head -20

# Or read via the MCP lisa_memory_dump tool which returns formatted hex text
```

### 7.2 find Command

```bash
echo "find 4e 73" | nc -w 2 127.0.0.1 1983
```
- Space-separated hex bytes (no 0x prefix)
- Returns first match address or "not found"

### 7.3 Physical Address Mapping

The Lisa MMU translates logical addresses through Segment Origin Registers (SOR):
- Physical address = (SOR × 512) + page_offset
- Virtual segment = virt_addr >> 17
- Page within segment = (virt_addr >> 9) & 0xFF
- Byte offset = virt_addr & 0x1FF

For supervisor code (kernel/ROM), the MMU context is typically 0 (OS context).
MMU SOR/SLR registers written to the MMU are logged as `[xenix-mmu-sor]`/`[xenix-mmu-slr]`
in the console log when Xenix compat logging is enabled.

### 7.4 Lisa RAM Layout (2MB config)

| Physical Range | Contents |
|----------------|----------|
| `0x000000–0x001FFF` | Interrupt vector table + kernel low memory |
| `0x002000–0x007FFF` | Kernel code (loaded from ProFile) |
| `0x007FF8–0x00FFFF` | VRAM (90 bytes/row × 364 rows = 32760 bytes) |
| `0x010000–0x1FFFFF` | Kernel heap, stack, user space |
| `0x200000+` | Unmapped — bus error on access |

---

## 8. Screenshot

### 8.1 Via Socket Command

```bash
echo "screenshot" | nc -w 5 127.0.0.1 1983
# Response: /path/to/.tmp/lisaem-screenshot.png
# Then use Read tool to view the PNG image
```

The socket `screenshot` command renders the emulated display using wxImage (upscaled).

### 8.2 Via MCP Tool (more reliable)

The `lisa_screenshot` MCP tool reads VRAM directly:
1. Gets `vram=` address from `status`
2. Dumps 90×364 = 32760 bytes from that address
3. Returns an inline 1-bit PNG image (720×364, black-on-white)

VRAM format:
- 364 rows × 90 bytes/row
- 1 bit per pixel, MSB-first (bit 7 = leftmost pixel)
- 1 = black pixel

### 8.3 Reading the Display

Use the Read tool on `.tmp/lisaem-screenshot.png` to visually inspect the current Lisa screen.
This is essential for:
- Verifying boot stage reached (ROM menu / boot: prompt / login prompt)
- Reading error messages from the OS
- Confirming mouse position

---

## 9. Console Log (Xenix Diagnostics)

The file `.tmp/lisaem-xenix-console.txt` is written by the emulator's Xenix compatibility
logging code. It contains:

### 9.1 ProFile State Machine Events

| Tag | Meaning |
|-----|---------|
| `[xenix-pf-ff]` | state2: 0xFF→0x55 coercion for Xenix compat |
| `[xenix-pf-2to4]` | state2→state4 transition, BSYLine reset |
| `[xenix-pf-4]` | Entered state4, BSYLine and write index |
| `[xenix-pf-bsy]` | state4b: BSYLine reset for IRQ edge |
| `[xenix-pf-4b*]` | state4b details |

### 9.2 Exception / Vector Events

| Tag | Meaning |
|-----|---------|
| `[xenix-vector] v=N pc=... addr=...` | 68000 exception vector N fired |
| `Supervisor Trap N` + register dump | Full register state at exception |

Common vector numbers:
- v=2: Bus error (access to unmapped memory)
- v=3: Address error (odd address access)
- v=11 (0xB): F-line emulator trap
- v=25: Level 1 IRQ (hard disk, floppy, video)
- v=26: Level 2 IRQ (keyboard, mouse, RTC)

### 9.3 HLE (High-Level Emulation) Events

| Tag | Meaning |
|-----|---------|
| `[xenix-hle] apply pc=000241ac context=1 trap=0000e3e8` | Bootloader→kernel handoff |
| `[xenix-sentinel]` | lisa_rl_ram overflow: physical address > maxlisaram |
| `[xenix-mmu-sor]` | MMU SOR register written (segment→physical mapping) |
| `[xenix-mmu-slr]` | MMU SLR register written (segment limit + permissions) |
| `[xenix-oflow]` | Physical address overflow in rl_ram (bus error triggered) |

### 9.4 Reading the Log

```bash
# Tail the log (last 100 lines):
tail -100 /Users/peter/GitHub/lisaem/bin/LisaEm.app/Contents/MacOS/.tmp/lisaem-xenix-console.txt

# Watch for exceptions:
grep "xenix-vector" /path/to/.tmp/lisaem-xenix-console.txt | tail -30

# Watch for register dumps after trap:
grep -A 3 "Supervisor Trap" /path/to/.tmp/lisaem-xenix-console.txt | tail -30
```

---

## 10. Status Monitoring Workflow

For debugging a Xenix boot, this sequence gives maximum visibility:

```bash
TMPDIR=/Users/peter/GitHub/lisaem/bin/LisaEm.app/Contents/MacOS/.tmp
SOCKET="127.0.0.1:1983"

# 1. Check emulator is alive
echo "status" | nc -w 2 $SOCKET

# 2. Capture current state
echo "status" | nc -w 2 $SOCKET | grep -E "^(pc|power|running|copsqueuelen)="

# 3. Dump exception vectors
echo "dump 0 400" | nc -w 2 $SOCKET
xxd $TMPDIR/lisaem-mcp-dump.bin | head -32

# 4. Check the console log
wc -l $TMPDIR/lisaem-xenix-console.txt
tail -50 $TMPDIR/lisaem-xenix-console.txt

# 5. Take a screenshot
echo "screenshot" | nc -w 5 $SOCKET
# Then: Read tool on $TMPDIR/lisaem-screenshot.png
```

---

## 11. Building LisaEm

```bash
cd /Users/peter/GitHub/lisaem
./build.sh clean build
```

- Build takes ~40 seconds on macOS ARM
- Output binary: `bin/LisaEm.app/Contents/MacOS/lisaem-arm64-26.03-wx3.2.9`
- wxWidgets 3.2.9 must already be built (in `/Users/peter/GitHub/wxWidgets-3.2.9/`)
- Check `obj/build-warnings.txt` for compiler warnings after build

After a code change, always kill the running emulator, rebuild, and relaunch before testing.

---

## 12. Diagnostic Code Hooks (active in current binary)

The following diagnostic logging is compiled into the current binary:

### 12.1 memory.c overflows

When `lisa_rl_ram` / `lisa_rw_ram` / `lisa_rb_ram` is called with a physical address
beyond `maxlisaram`, instead of returning the sentinel 0x39393939, it:
1. Logs `[xenix-oflow]` (first 20 occurrences)
2. Calls `lisa_mmu_exception(addr)` to generate a proper 68000 bus error

### 12.2 cpu68k.c basic block overflow

If the 68000 JIT decoder encounters a basic block > 8192 instructions (e.g. CPU running
through unmapped memory), it truncates the block with a warning instead of calling exit().
Log line: `WARNING: basic block too large (N instrs) at pc=XXXXXXXX - truncating`

### 12.3 ProFile state machine

In `src/storage/profile.c`:
- state2→4: BSYLine reset to 0 so state4 generates a CA1 IRQ edge
- state4b: BSYLine reset to 0 for bootloader CA1 IRQ edge
- state10: CMDLine check bypassed for Xenix compat so data phase reads work

### 12.4 reg68k.c vector logging

Every 68000 exception logs to the console file as:
```
[xenix-vector] v=N pc=XXXXXXXX addr=XXXXXXXX sr=XXXX sp=XXXXXXXX [d0=... d1=...]
```
Level 1/2 IRQs (v=25/26) are logged at most once per 5M CPU cycles to reduce noise.

---

## 13. Known Gotchas

1. **apple3 timing**: The ROM boot menu needs ~7 full seconds of emulated time before it
   will accept the Apple+3 boot selection. Send it too early → silent consumption, no boot.

2. **force-shutdown** stops the emulated Lisa but does NOT quit the emulator process.
   This is correct; use it to get back to a clean state before re-powering.

3. **dump address space**: The `dump` command works on **physical** RAM addresses
   (0 to maxlisaram-1). Virtual/logical addresses must be translated via the MMU SOR
   registers first. The Xenix kernel runs with MMU contexts active.

4. **screenshot requires power-on**: The `screenshot` socket command and MCP screenshot
   tool both need the Lisa to be powered on with a valid VRAM address. Check `power=1`
   and `vram!=00000000` in the status response first.

5. **Console log only appears after Xenix starts**: The `.tmp/lisaem-xenix-console.txt`
   file is created on the first `xenix_log_note()` call. It won't exist during the ROM
   boot menu phase.

6. **Serial injection** (`serial_a` command): Injects directly into the Z8530 receive buffer.
   This bypasses any handshaking. Useful for sending commands to a serial-attached program,
   but use with care — the emulated Z8530 must be in a receiving state.

7. **Text queue** is only 256 bytes. For long strings, use multiple `text` commands with
   `wait` delays between them: `text part1\nwait 5\ntext part2\n`.

8. **PC values in ROM** (0xFE0000–0xFFFFFF) indicate the Lisa is in the boot ROM.
   PC values in the range 0x00000288–0x001FFFFF indicate kernel/bootloader execution.
   PC values in 0x200000–0xFDFFFF indicate an unmapped memory hole (crash likely).
