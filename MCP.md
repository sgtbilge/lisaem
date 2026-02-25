# LisaEm Native MCP (stdio) - WIP

This branch includes a native MCP server inside LisaEm using stdio JSON-RPC framing.

## Launch

Run LisaEm from a terminal with `--mcp-stdio` so an MCP client can own stdin/stdout:

```bash
./bin/LisaEm.app/Contents/MacOS/lisaem.sh --mcp-stdio -c lisaem.conf
```

Notes:

- MCP messages are served on `stdin/stdout` (Content-Length framing).
- The legacy raw command socket remains available on `127.0.0.1:1983`.
- In `--mcp-stdio` mode, LisaEm redirects accidental stdout output to `/dev/null` and uses a dedicated fd for MCP responses.

## Implemented MCP methods (phase 1)

- `initialize`
- `ping`
- `tools/list`
- `tools/call`

## Tool names (phase 1)

- `lisa_status`
- `lisa_power`
- `lisa_run_state`
- `lisa_input_text`
- `lisa_input_keycode`
- `lisa_input_mouse`
- `lisa_shortcut`
- `lisa_memory_dump`
- `lisa_memory_find`
- `lisa_debug_status`
- `lisa_debug_command`
- `lisa_xenix_console_tail`
- `lisa_xenix_wait_console`
- `lisa_capture_trap_snapshot`

## Xenix troubleshooting workflow (recommended)

1. Use `lisa_status` to confirm the emulator is running and powered.
2. Drive installer/boot interaction with:
   - `lisa_input_text`
   - `lisa_input_mouse`
   - `lisa_shortcut` (`apple2`, `return`, etc.)
3. Observe progress with:
   - `lisa_xenix_console_tail`
   - `lisa_xenix_wait_console` (polling helper; non-blocking phase 1 behavior)
4. Inspect traps/failures with:
   - `lisa_debug_status`
   - `lisa_debug_command` (`pause`, `step N`, `mark`, `rewind`, `findtext`, `watchtext`, ...)
   - `lisa_capture_trap_snapshot`

## Current limitations

- Polling-only (no MCP notifications/subscriptions yet)
- `lisa_xenix_wait_console` is implemented as a polling helper (it does not block the in-process emulation loop waiting for future output)
- JSON handling is intentionally minimal and targeted at MCP request/response/tool calls used by CLI clients
