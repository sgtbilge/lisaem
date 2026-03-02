#!/usr/bin/env bash
# Automated Xenix boot test: kill → launch → boot → monitor
set -uo pipefail

LISAEM_DIR="/Users/peter/GitHub/lisaem"
MACOSDIR="$LISAEM_DIR/bin/LisaEm.app/Contents/MacOS"
TMPDIR_INNER="$MACOSDIR/.tmp"
CONSOLE_LOG="$TMPDIR_INNER/lisaem-xenix-console.txt"

send() { echo "$1" | nc -w 2 127.0.0.1 1983 2>/dev/null | grep "^message="; }
pc()   { echo "status" | nc -w 1 127.0.0.1 1983 2>/dev/null | grep "^pc=" | cut -d= -f2; }
wait_socket() {
    for i in $(seq 1 30); do
        echo "status" | nc -w 1 127.0.0.1 1983 2>/dev/null | grep -q "^running=" && return 0
        sleep 1; printf "."
    done
    echo "TIMEOUT waiting for socket"; return 1
}

# ── 1. Kill any existing LisaEm ──────────────────────────────────────────────
echo "Killing any existing LisaEm..."
pkill -x "lisaem-arm64-26.03-wx3.2.9" 2>/dev/null || true
sleep 2

# ── 2. Clear logs ─────────────────────────────────────────────────────────────
mkdir -p "$TMPDIR_INNER"
rm -f "$CONSOLE_LOG"

# ── 3. Launch with -p (auto power-on) and -d (auto-boot from hard disk) ───────
# These flags fire apple_3() internally at the exact right CPU clock window
# (15M-30M cycles), bypassing the unreliable socket-based apple3 timing.
echo "Launching LisaEm with -p -d (auto power + auto ProFile boot)..."
cd "$MACOSDIR"
export LISAEM_TMPDIR="$TMPDIR_INNER"
nohup ./lisaem-arm64-26.03-wx3.2.9 -p -d >/dev/null 2>&1 &
LISAEM_PID=$!
cd "$LISAEM_DIR"
echo "PID: $LISAEM_PID"

# ── 4. Wait for socket ────────────────────────────────────────────────────────
printf "Waiting for socket"; wait_socket; echo " ready"

# ── 5. Wait for PC to enter bootloader range (0x00020000–0x00025000) ─────────
echo "Waiting for bootloader..."
for i in $(seq 1 30); do
    sleep 1
    CUR_PC=$(pc)
    printf "  PC=0x%s\n" "$CUR_PC"
    if [[ "$CUR_PC" > "00020000" && "$CUR_PC" < "00025000" ]]; then
        echo "  Bootloader active at PC=0x$CUR_PC"
        break
    fi
done
sleep 2

# ── 10. Monitor console for up to 30s ────────────────────────────────────────
echo "Watching console log..."
for i in $(seq 1 30); do
    sleep 1
    LINES=$(wc -l < "$CONSOLE_LOG" 2>/dev/null | tr -d ' ')
    CUR_PC=$(pc)
    printf "  t+%2ds PC=0x%s  log=%s lines\n" "$i" "$CUR_PC" "${LINES:-0}"
    # Stop early if log has grown past the initial pf-ff block
    if [[ "${LINES:-0}" -gt 12 ]]; then
        echo "  Log growing — stopping early"
        break
    fi
done

# ── 11. Print final console ───────────────────────────────────────────────────
echo ""
echo "=== CONSOLE LOG ==="
cat "$CONSOLE_LOG" 2>/dev/null || echo "(empty)"
echo ""
echo "=== STATUS ==="
echo "status" | nc -w 2 127.0.0.1 1983 2>/dev/null
