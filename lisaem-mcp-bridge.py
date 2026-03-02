#!/usr/bin/env python3
"""
LisaEm MCP Bridge
Exposes LisaEm's control socket (127.0.0.1:1983) and file-based state
as an MCP server for Claude Code. Run LisaEm normally yourself; this
bridge is the subprocess that Claude Code owns.
"""
import sys, os, json, socket, select, time

TMPDIR = os.environ.get("LISAEM_TMPDIR",
         os.path.join(os.path.dirname(os.path.abspath(__file__)), ".tmp"))
SOCK_HOST = "127.0.0.1"
SOCK_PORT = 1983


# ── socket helpers ────────────────────────────────────────────────────────────

def _sock_send_recv(cmd: str, timeout: float = 3.0) -> str:
    """Send one line to LisaEm's socket, return the response block."""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(timeout)
        s.connect((SOCK_HOST, SOCK_PORT))
        s.sendall((cmd.strip() + "\n").encode())
        data = b""
        while True:
            r, _, _ = select.select([s], [], [], timeout)
            if not r:
                break
            chunk = s.recv(4096)
            if not chunk:
                break
            data += chunk
            # Response always ends at the "mouse=" line
            if b"mouse=" in data and data.rstrip().endswith((b"\n", b"\r")):
                break
        s.close()
        return data.decode(errors="replace").strip()
    except ConnectionRefusedError:
        return "error: LisaEm not running (socket refused on port 1983)"
    except Exception as e:
        return f"error: {e}"


def _read_file_tail(filename: str, max_bytes: int = 16384) -> str:
    path = os.path.join(TMPDIR, filename)
    try:
        with open(path, "rb") as f:
            f.seek(0, 2)
            size = f.tell()
            f.seek(max(0, size - max_bytes))
            return f.read().decode(errors="replace")
    except FileNotFoundError:
        return f"(file not found: {path})"
    except Exception as e:
        return f"(error reading {path}: {e})"


def _hex_dump(data: bytes, base_addr: int) -> str:
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        hex_part = " ".join(f"{b:02x}" for b in chunk)
        asc_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        lines.append(f"{base_addr + i:08x}: {hex_part:<48}  {asc_part}")
    return "\n".join(lines)


# ── tool implementations ──────────────────────────────────────────────────────

def _tool_status(_args):
    return _sock_send_recv("status")

def _tool_power(_args):
    return _sock_send_recv("power")

def _tool_run_state(args):
    action = args.get("action", "status")
    if action not in ("pause", "run"):
        return "error: action must be pause or run"
    return _sock_send_recv(action)

def _tool_input_text(args):
    text = args.get("text", "")
    # socket 'text' command queues chars + appends \r automatically
    return _sock_send_recv(f"text {text}")

def _tool_shortcut(args):
    name = args.get("name", "")
    allowed = {"apple1","apple2","apple3","return","shutdown","quit","focus",
               "power","pause","run","force-shutdown"}
    if name not in allowed:
        return f"error: unknown shortcut '{name}'. Allowed: {', '.join(sorted(allowed))}"
    return _sock_send_recv(name)

def _tool_input_mouse(args):
    action = args.get("action", "move")
    x, y = int(args.get("x", 0)), int(args.get("y", 0))
    if action not in ("move", "click"):
        return "error: action must be move or click"
    return _sock_send_recv(f"{action} {x} {y}")

def _tool_memory_dump(args):
    addr_s = args.get("addr_hex", "0")
    len_s  = args.get("len_hex", "40")
    cx_s   = args.get("context", None)
    if cx_s is not None:
        cmd = f"dump context {cx_s} {addr_s} {len_s}"
    else:
        cmd = f"dump {addr_s} {len_s}"
    result = _sock_send_recv(cmd)
    dump_path = os.path.join(TMPDIR, "lisaem-mcp-dump.bin")
    try:
        with open(dump_path, "rb") as f:
            data = f.read()
        return result + "\n" + _hex_dump(data, int(addr_s, 16))
    except Exception:
        return result

def _tool_memory_find(args):
    raw = args.get("pattern_hex", "").replace(" ", "")
    pairs = " ".join(raw[i:i+2] for i in range(0, len(raw), 2))
    cx_s = args.get("context", None)
    if cx_s is not None:
        cmd = f"find context {cx_s} {pairs}"
    else:
        cmd = f"find {pairs}"
    return _sock_send_recv(cmd)

def _tool_xenix_console_tail(args):
    max_bytes = int(args.get("max_bytes", 16384))
    return _read_file_tail("lisaem-xenix-console.txt", max_bytes)

def _tool_debug_command(args):
    cmd = args.get("command", "status")
    return _sock_send_recv(cmd)

def _parse_response(resp: str) -> dict:
    result = {}
    for line in resp.splitlines():
        if '=' in line:
            k, _, v = line.partition('=')
            result[k.strip()] = v.strip()
    return result


def _make_1bit_png(raw_vram: bytes, width: int = 720, height: int = 364) -> bytes:
    """Wrap raw Lisa VRAM (1=black, MSB-first, 90 bytes/row) in a 1-bit grayscale PNG."""
    import struct, zlib

    def chunk(tag, data):
        c = tag + data
        return struct.pack('>I', len(data)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)

    ihdr = chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 1, 0, 0, 0, 0))

    bytes_per_row = (width + 7) // 8   # 720 / 8 = 90
    rows = bytearray()
    for y in range(height):
        rows.append(0x00)              # PNG filter type: None
        row = raw_vram[y * bytes_per_row : (y + 1) * bytes_per_row]
        rows.extend(b ^ 0xFF for b in row)  # invert: VRAM 1=black → PNG 1=white

    idat = chunk(b'IDAT', zlib.compress(bytes(rows), 9))
    iend = chunk(b'IEND', b'')
    return b'\x89PNG\r\n\x1a\n' + ihdr + idat + iend


def _tool_screenshot(_args):
    import base64
    status_resp = _sock_send_recv("status")
    parsed = _parse_response(status_resp)
    vram_addr = parsed.get("vram", "")
    if not vram_addr or vram_addr == "00000000":
        return f"error: VRAM address unavailable (Lisa powered off?)\n{status_resp}"

    # 90 bytes/row × 364 rows = 32760 = 0x7FF8
    dump_resp = _sock_send_recv(f"dump {vram_addr} 7FF8")
    dump_path = os.path.join(TMPDIR, "lisaem-mcp-dump.bin")
    try:
        with open(dump_path, "rb") as f:
            raw_vram = f.read()
    except Exception as e:
        return f"error reading VRAM dump: {e}\n{dump_resp}"

    if len(raw_vram) < 32760:
        return f"error: expected 32760 bytes, got {len(raw_vram)}\n{dump_resp}"

    png = _make_1bit_png(raw_vram[:32760])
    return [
        {"type": "text",  "text": f"Lisa display captured: 720×364 1-bit PNG ({len(png)} bytes) from VRAM @ 0x{vram_addr}"},
        {"type": "image", "data": base64.b64encode(png).decode(), "mimeType": "image/png"},
    ]


def _tool_trap_snapshot(_args):
    status  = _sock_send_recv("status")
    console = _read_file_tail("lisaem-xenix-console.txt", 6144)
    state   = _read_file_tail("lisaem-mcp-state.txt", 2048)
    return (f"=== STATUS ===\n{status}\n\n"
            f"=== MCP STATE ===\n{state}\n\n"
            f"=== CONSOLE TAIL ===\n{console}")

DISPATCH = {
    "lisa_status":              _tool_status,
    "lisa_power":               _tool_power,
    "lisa_run_state":           _tool_run_state,
    "lisa_input_text":          _tool_input_text,
    "lisa_shortcut":            _tool_shortcut,
    "lisa_input_mouse":         _tool_input_mouse,
    "lisa_memory_dump":         _tool_memory_dump,
    "lisa_memory_find":         _tool_memory_find,
    "lisa_xenix_console_tail":  _tool_xenix_console_tail,
    "lisa_debug_command":       _tool_debug_command,
    "lisa_capture_trap_snapshot": _tool_trap_snapshot,
    "lisa_screenshot":            _tool_screenshot,
}

TOOLS = [
    {"name":"lisa_status","description":"Get LisaEm emulator state (power, PC, clock, mouse)",
     "inputSchema":{"type":"object","properties":{},"additionalProperties":False}},
    {"name":"lisa_power","description":"Toggle Lisa power button (on→off or off→on)",
     "inputSchema":{"type":"object","properties":{},"additionalProperties":False}},
    {"name":"lisa_run_state","description":"Pause or resume the emulation loop",
     "inputSchema":{"type":"object","properties":{"action":{"type":"string","enum":["pause","run"]}},"required":["action"],"additionalProperties":False}},
    {"name":"lisa_input_text","description":"Type ASCII text into the emulator (queued, ~200 ms/char)",
     "inputSchema":{"type":"object","properties":{"text":{"type":"string"}},"required":["text"],"additionalProperties":False}},
    {"name":"lisa_shortcut","description":"Send a named shortcut: apple1 apple2 apple3 return shutdown quit focus power pause run force-shutdown",
     "inputSchema":{"type":"object","properties":{"name":{"type":"string"}},"required":["name"],"additionalProperties":False}},
    {"name":"lisa_input_mouse","description":"Move or click the emulated mouse at pixel coordinates",
     "inputSchema":{"type":"object","properties":{"action":{"type":"string","enum":["move","click"]},"x":{"type":"integer"},"y":{"type":"integer"}},"required":["action","x","y"],"additionalProperties":False}},
    {"name":"lisa_memory_dump","description":"Dump Lisa RAM as hex. addr_hex and len_hex are hex strings (e.g. '00020000','80'). optional context is a hex string.",
     "inputSchema":{"type":"object","properties":{"addr_hex":{"type":"string"},"len_hex":{"type":"string"},"context":{"type":"string"}},"required":["addr_hex","len_hex"],"additionalProperties":False}},
    {"name":"lisa_memory_find","description":"Search Lisa RAM for a hex byte pattern. optional context is a hex string.",
     "inputSchema":{"type":"object","properties":{"pattern_hex":{"type":"string"},"context":{"type":"string"}},"required":["pattern_hex"],"additionalProperties":False}},
    {"name":"lisa_xenix_console_tail","description":"Read the Xenix HLE console log (last max_bytes bytes)",
     "inputSchema":{"type":"object","properties":{"max_bytes":{"type":"integer"}},"additionalProperties":False}},
    {"name":"lisa_debug_command","description":"Send a raw socket command to LisaEm (status|pause|run|dump|find|text|click|etc.)",
     "inputSchema":{"type":"object","properties":{"command":{"type":"string"}},"required":["command"],"additionalProperties":False}},
    {"name":"lisa_capture_trap_snapshot","description":"Collect status + MCP state + Xenix console tail for crash/trap analysis",
     "inputSchema":{"type":"object","properties":{},"additionalProperties":False}},
    {"name":"lisa_screenshot","description":"Capture the current Lisa display as a 720×364 1-bit PNG read directly from emulated VRAM. Returns the image for visual inspection.",
     "inputSchema":{"type":"object","properties":{},"additionalProperties":False}},
]


# ── MCP stdio framing ─────────────────────────────────────────────────────────

def _read_msg():
    content_length = None
    while True:
        line = sys.stdin.buffer.readline()
        if not line:
            return None
        line = line.strip()
        if line.lower().startswith(b"content-length:"):
            content_length = int(line.split(b":", 1)[1].strip())
        elif line == b"" and content_length is not None:
            body = sys.stdin.buffer.read(content_length)
            return json.loads(body)


def _write_msg(obj):
    body = json.dumps(obj).encode()
    header = f"Content-Length: {len(body)}\r\n\r\n".encode()
    sys.stdout.buffer.write(header + body)
    sys.stdout.buffer.flush()


def main():
    while True:
        msg = _read_msg()
        if msg is None:
            break
        method = msg.get("method", "")
        mid    = msg.get("id")

        if method == "initialize":
            _write_msg({"jsonrpc":"2.0","id":mid,"result":{
                "protocolVersion":"2024-11-05",
                "capabilities":{"tools":{"listChanged":False}},
                "serverInfo":{"name":"lisaem-bridge","version":"1.0"}
            }})

        elif method in ("notifications/initialized", "notifications/cancelled"):
            pass  # no response for notifications

        elif method == "ping":
            _write_msg({"jsonrpc":"2.0","id":mid,"result":{}})

        elif method == "tools/list":
            _write_msg({"jsonrpc":"2.0","id":mid,"result":{"tools":TOOLS}})

        elif method == "tools/call":
            params    = msg.get("params", {})
            tool_name = params.get("name", "")
            tool_args = params.get("arguments", {})
            fn = DISPATCH.get(tool_name)
            if fn:
                try:
                    result = fn(tool_args)
                except Exception as e:
                    result = f"error: {e}"
            else:
                result = f"unknown tool: {tool_name}"
            content = result if isinstance(result, list) else [{"type":"text","text":str(result)}]
            _write_msg({"jsonrpc":"2.0","id":mid,"result":{"content":content}})

        elif mid is not None:
            _write_msg({"jsonrpc":"2.0","id":mid,"error":{
                "code":-32601,"message":f"Unknown method: {method}"
            }})


if __name__ == "__main__":
    main()
