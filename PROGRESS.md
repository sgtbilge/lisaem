# LisaEm Xenix Boot Debug — Progress Tracker

## Goal
Get Xenix to boot past `pfcmd: handshake err 0102 × 11` → login prompt.

## Conventions (STABLE — do not change)
- **Binary**: `bin/LisaEm.app/Contents/MacOS/lisaem-arm64-26.03-wx3.2.9`
- **Working dir for launch**: `bin/LisaEm.app/Contents/MacOS/`
- **TMPDIR**: `bin/LisaEm.app/Contents/MacOS/.tmp/`
- **Socket**: `127.0.0.1:1983`
- **Console log**: `.tmp/lisaem-xenix-console.txt`

## Key Keycodes (use `keycode raw`, NOT `apple3`/`return` socket commands)
- Apple+3 (boot from ProFile):
  ```
  keycode raw 7f 80   # Command down
  keycode raw 72 80   # 3 down
  keycode raw 72 00   # 3 up
  keycode raw 7f 00   # Command up
  ```
- Return:
  ```
  keycode raw 48 80   # Return down
  keycode raw 48 00   # Return up
  ```
- Why: `apple_combo_key()` has a queue-full guard that silently drops keys.
  `keycode raw` calls `send_cops_keycode()` directly, bypassing it.

## Boot Sequence (manual steps until automated)
1. Launch: `LISAEM_TMPDIR=$(pwd)/.tmp nohup ./lisaem-arm64-26.03-wx3.2.9 &`
2. Wait for socket: `echo "status" | nc -w 2 127.0.0.1 1983`
3. Power on: `echo "power" | nc -w 2 127.0.0.1 1983`
4. Wait ~6s for ROM boot menu (PC = 0xfe2dce)
5. Send Apple+3 via keycode raw (PC should jump to 0x00022d4c = bootloader)
6. Send Return via keycode raw
7. Monitor console log and PC

## Fixes Implemented

### Fix 1: profile.c state2→4 BSYLine edge (DONE, built)
- **Problem**: BSYLine stays=1 from state2 into state4, so no 0→1 edge is generated → CA1 IRQ never fires → bootloader never sends command bytes.
- **Fix**: Reset `P->BSYLine=0` at state2→4 transition when compat enabled.
- **File**: `src/storage/profile.c` ~line 1042

### Fix 2: profile.c state4b BSYLine reset (DONE, built)
- **Problem**: Similar issue in state4b for READ commands.
- **Fix**: Reset `P->BSYLine=0` in state4b case 0 when compat + ROM running.
- **File**: `src/storage/profile.c` (state 4b)

### Fix 3: apple_combo_key silent drop (DIAGNOSED, workaround applied)
- **Problem**: `apple_combo_key()` silently drops key if COPS queue has room < size.
- **Workaround**: Use `keycode raw` socket command instead of `apple3`.
- **Permanent fix needed**: Change `apple3` socket command to use `send_cops_keycode()` directly.
- **File**: `src/host/wxui/lisaem_wx.cpp` ~line 1150

## Pending Work

### TODO 1: Screenshot socket command (PLANNED, NOT YET IMPLEMENTED)
- Plan exists at `/Users/peter/.claude/plans/lazy-squishing-russell.md`
- Add `screenshot` to socket command dispatcher in `lisaem_wx.cpp` ~line 1283
- Add `lisa_screenshot` tool to `lisaem-mcp-bridge.py`
- Needed for: seeing what's on screen without physical access to the window

### TODO 2: Confirm Return key reaches Xenix bootloader
- **Status**: BLOCKED — Return via `keycode raw 48 80/00` sent but PC stays at 0x22d46-0x22d50
- **Unknown**: Is the bootloader (a) waiting for Return to start ProFile comms, or (b) already in a ProFile polling loop?
- **Need**: Screenshot to see current screen state, OR disassembly of 0x22d46-0x22d50

### TODO 3: Verify state machine fixes work end-to-end
- Once bootloader is triggered: confirm `[xenix-pf-2to4]`, `[xenix-pf-4]`, `[xenix-pf-bsy]` appear in console log
- If state4b reached → Xenix should start loading

### TODO 4: Automate full boot in run_xenix_test.sh
- Fix the script to use `keycode raw` for Apple+3 and Return
- Add ROM menu detection before sending keys
- Add ProFile state monitoring

## Diagnostic Logs (in .tmp/lisaem-xenix-console.txt)
- `[xenix-pf-ff]` — state2 compat: coerced 0xff→0x55
- `[xenix-pf-2to4]` — state2→4 transition, BSY reset to 0
- `[xenix-pf-4]` — entered state4, BSYLine and idxw values
- `[xenix-pf-bsy]` — state4b: BSY reset to 0 for IRQ edge
- `[xenix-pf-4b*]` — state4b details

## Current Session State
- LisaEm running, PID 27023
- Apple+3 sent successfully (PC jumped to 0x22d4c)
- Return sent but PC stuck at 0x22d46-0x22d50
- Console log: not yet created (ProFile state machine not triggered)
- **Next**: Take screenshot to see what bootloader is displaying
