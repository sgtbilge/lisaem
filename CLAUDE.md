# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LisaEm is a full-featured emulator of the Apple Lisa computer (1983), predecessor to the Macintosh. The emulator faithfully recreates the Lisa hardware including CPU, memory, I/O devices, video, and storage subsystems. It runs on multiple platforms via wxWidgets.

**Key Info:**
- Version: 1.2.7 RC4 (2022.04.01)
- License: GNU GPL v3
- Language: C/C++
- UI Framework: wxWidgets 3.0.4 - 3.1.5
- Build System: bashbuild (custom shell-based system)

## Build Commands

### Building wxWidgets (Required First Step)

LisaEm requires a custom-built wxWidgets installation. Use the scripts in `scripts/`:

```bash
# For modern macOS:
cd scripts
./build-wx3.1.5-modern-macosx.sh

# For Linux/GTK:
./build-wx3.1.5-gtk.sh

# For Windows (Cygwin):
./build-wx3.1.5-cygwin-windows.sh

# Add wx-config to PATH after building:
export PATH=/usr/local/wx3.1.5-cocoa-x64-macOS-10.15/bin:$PATH
```

**Important:** Do not use system-provided wxWidgets packages. Build your own using the provided scripts.

### Building LisaEm

```bash
# Clean and build:
./build.sh clean build

# Install (requires sudo on Unix, Administrator on Windows):
sudo ./build.sh install

# Build with debug symbols and trace logging:
./build.sh clean build --with-debug --with-tracelog

# Build 64-bit specifically:
./build.sh clean build --64

# Cross-compile for different architecture:
./build.sh clean build -arch=ppc
```

**Note:** The `./configure` and `Makefile` are wrappers around `build.sh` for compatibility with traditional build systems.

### Build Output Locations

- Binaries: `bin/lisaem` (main emulator)
- Tools: `bin/lisafsh-tool`, `bin/dc42-*`, etc.
- Objects: `obj/`
- Packages: `pkg/`
- Build warnings: `obj/build-warnings.txt`

### Building Individual Components

```bash
# Build just libdc42:
cd src/lib/libdc42
./build.sh build

# Build just the tools:
cd src/tools
./build.sh build

# Build from any subdirectory (goes up to TLD):
cd src/host/wxui
./build.sh build
```

## Code Architecture

### High-Level Structure

```
┌─────────────────────────────────────┐
│  Host Platform Layer (wxUI)         │  - wxWidgets GUI
│  src/host/wxui/                     │  - Input/display/events
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  Emulation Core                     │  - CPU cycle timing
│  src/lisa/motherboard/              │  - Event dispatch
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  Hardware Components                │
│  - CPU Board (68000, MMU, RAM)      │  src/lisa/cpu_board/
│  - I/O Board (serial, floppy, VIA)  │  src/lisa/io_board/
│  - Video Board (display)            │  src/lisa/video_board/
│  - Storage (ProFile, Widget)        │  src/storage/
└─────────────────────────────────────┘
```

### Key Directories

| Path | Purpose |
|------|---------|
| `src/lisa/cpu_board/` | 68000 CPU, MMU, memory management, ROM, interrupts |
| `src/lisa/io_board/` | VIA6522, Z8530 serial, floppy controller, COPS |
| `src/lisa/motherboard/` | System integration, glue logic, event dispatch |
| `src/storage/` | ProFile/Widget hard drive emulation |
| `src/host/wxui/` | wxWidgets UI layer, main frame, config dialogs |
| `src/lib/libGenerator/` | 68000 CPU emulation core (from Generator) |
| `src/lib/libdc42/` | Disk image format library (DiskCopy 4.2) |
| `src/lib/TerminalWx/` | VT100 terminal widget |
| `src/tools/` | Disk image manipulation utilities (~20 tools) |
| `src/include/` | Global headers and definitions |
| `bashbuild/` | Build system library functions |

### Critical Files Reference

**CPU & Memory:**
- `src/lisa/cpu_board/memory.c` (4,141 lines) - Memory access abstraction
- `src/lisa/cpu_board/mmu.c` (1,156 lines) - MMU with context switching
- `src/lisa/cpu_board/irq.c` (982 lines) - Interrupt handling
- `src/lisa/cpu_board/rom.c` (640 lines) - ROM loading
- `src/lisa/cpu_board/romless.c` (996 lines) - Boot from disk support

**I/O Devices:**
- `src/lisa/io_board/via6522.c` (3,001 lines) - Versatile Interface Adapter
- `src/lisa/io_board/z8530.c` (1,829 lines) - Serial controller
- `src/lisa/io_board/floppy.c` (1,999 lines) - Floppy drive controller
- `src/lisa/io_board/cops.c` (1,261 lines) - Power/clock/keyboard

**System Integration:**
- `src/lisa/motherboard/glue.c` (753 lines) - System integration
- `src/lisa/motherboard/symbols.c` (2,081 lines) - LOS error codes
- `src/lisa/motherboard/fliflo_queue.c` (338 lines) - Event queue

**Host UI:**
- `src/host/wxui/lisaem_wx.cpp` (10,514 lines) - Main application
- `src/host/wxui/LisaConfig.cpp` (292 lines) - Configuration storage
- `src/host/wxui/LisaConfigFrame.cpp` (922 lines) - Configuration UI
- `src/host/wxui/z8530-terminal.cpp` (1,110 lines) - Terminal widget

**Global State:**
- `src/include/vars.h` (129,928 bytes) - Global variables
- `src/machine.h` - Machine-specific type definitions

### Design Patterns

**Hardware Abstraction:**
- Memory access through function abstraction (`lisa_rb`, `lisa_rw`, `lisa_rl`, etc.)
- Device behavior encapsulated in separate files
- Event-driven timing simulation via event queue

**Configuration Management:**
- wxFileConfig for persistent storage
- Separate data model (LisaConfig) from UI (LisaConfigFrame)
- PRAM (Parameter RAM) preservation between sessions

**Display Rendering:**
- Dirty rectangle tracking for efficient updates
- Multiple scaling algorithms (HQx, direct pixel, AA, AAG)
- Configurable refresh rates (8-60 Hz)
- Support for fullscreen, skinned, and skinless modes

**I/O Flexibility:**
- Serial ports support multiple backends:
  - PTY (pseudo-terminal)
  - TTY (physical serial port)
  - TELNETD (network telnet server)
  - TERMINAL (wxWidgets terminal window)
  - IMAGEWRITER (printer emulation)
  - LOOPBACKPLUG (testing)

## Key Tools

All tools are built into `bin/` and work with DC42 disk images:

**Format Conversion:**
- `blu-to-dc42` - Convert BLU images (from physical Lisa) to DC42
- `dc42-to-blu` - Convert DC42 to BLU (for uploading to physical Lisa)
- `raw-to-dc42` - Convert raw binary images to DC42
- `dc42-to-raw` - Convert DC42 to raw format
- `idefile-to-dc42` - Convert IDE:File images to DC42

**Disk Manipulation:**
- `lisafsh-tool` - Interactive Lisa File System shell (with readline)
- `lisadiskinfo` - Display disk image information
- `dc42-diff` - Compare two disk images
- `dc42-dumper` - Dump disk contents to text files
- `dc42-add-tags` - Add missing tags to images
- `dc42-copy-boot-loader` - Copy boot sectors between images
- `dc42-copy-selected-sectors` - Copy specific sectors

**System Tools:**
- `lisa-serial-info` - Parse Lisa serial numbers
- `decode-vsrom` - Decode video state ROM serial numbers
- `los-deserialize` - Remove DRM from Lisa Office System tools
- `patchxenix` - Patch Xenix for X/ProFile compatibility
- `uniplus-bootloader-deserialize` - Parse UniPlus boot sectors

## Running LisaEm

```bash
# Launch GUI:
./bin/lisaem

# Command-line options:
./bin/lisaem --help
./bin/lisaem --power                    # Power on immediately
./bin/lisaem --quit                     # Quit after shutdown
./bin/lisaem --fullscreen               # Launch fullscreen
./bin/lisaem --zoom 2.0                 # Set zoom level
./bin/lisaem --config path/to/config    # Use specific config
./bin/lisaem --kiosk                    # Kiosk mode (for RPi)
```

## Common Development Workflows

### Making Changes to Emulation Core

1. Identify the affected component (CPU, MMU, I/O device, etc.)
2. Read the relevant source file in `src/lisa/`
3. Make changes
4. Rebuild: `./build.sh clean build --with-debug`
5. Test with debug logging enabled if needed

### Adding New Disk Image Tools

1. Create new `.c` file in `src/tools/src/`
2. Include `libdc42.h` header
3. Link against libdc42 library
4. Add to build list in `src/tools/build.sh`
5. Build: `cd src/tools && ./build.sh build`

### Modifying wxWidgets UI

1. Edit files in `src/host/wxui/`
2. Headers are in `src/host/wxui/include/`
3. Rebuild: `./build.sh clean build`
4. UI changes require full rebuild due to static resources

### Working with libdc42

1. Source: `src/lib/libdc42/src/`
2. Tests: `src/lib/libdc42/tester/`
3. Build library: `cd src/lib/libdc42 && ./build.sh build`
4. Run tests: `./bin/tester` (after building)

## Testing

**Test Locations:**
- `src/lisa/cpu_board/tests/` - MMU and CPU tests
- `src/lib/libdc42/tester/` - DC42 format validation tests

**Running Tests:**
```bash
# Build and run libdc42 tests:
cd src/lib/libdc42
./build.sh build
../../bin/tester

# MMU tests are standalone C files
cd src/lisa/cpu_board/tests
gcc -o mmu-test mmu-sor-test-add.c -I../../include
./mmu-test
```

**Note:** Test coverage is limited. Most testing is done manually with actual Lisa disk images and operating systems.

## Known Issues & Limitations

**Critical Bugs:**
- Windows: Unicode characters in disk image paths fail. Use ASCII-only paths.
- Serial port A causes lockups in Lisa Office System (disabled by default)
- TerminalWx: Closing window causes segfault
- Mouse tracking fails under high throttle (>48MHz on some systems)

**OS Compatibility:**
- ✓ Lisa Office System 3.x - Works well
- ✓ MacWorks XL 3.0 - Works (requires floppy boot)
- ✓ UniPlus v1.4 - Works from pre-installed images
- ✗ UniPlus fresh install - Fails (kernel v1.1 not supported)
- ✗ Xenix - Partially boots, fails before login prompt
- ✗ MacWorks XL 3.1+ - Does not boot

**Platform Issues:**
- macOS 10.8/10.9: UPX compression causes segfaults (disabled in build)
- GTK: Sound requires SDL + pulseaudio/ALSA proxy
- Cygwin: Must run build from Administrator shell for install

## Platform-Specific Notes

### macOS

**Requirements:**
- Xcode (version matching your OS)
- Command Line Tools
- For older OS X: `brew install gcc@7` (clang has issues)

**Building on older macOS:**
```bash
export PATH=/usr/local/wx3.1.2-cocoa-x64-macOS-10.11/bin:$PATH
CC=gcc-7 CXX=gcc-7 ./build.sh clean build --64
```

**wxWidgets Fix for macOS:**
If you get `'tr1/type_traits' file not found`, edit `wx-config --prefix`/include/wx-3.1/wx/strvararg.h and add around line 20:
```cpp
#include <ciso646>  // detect std::lib
#ifdef _LIBCPP_VERSION
#ifndef HAVE_TYPE_TRAITS
#define HAVE_TYPE_TRAITS 1
#endif
#else
#ifndef HAVE_TR1_TYPE_TRAITS
#define HAVE_TR1_TYPE_TRAITS 1
#endif
#endif
```

### Windows/Cygwin

**Requirements:**
- Cygwin with mingw-w64
- Use provided install scripts: `cygwin-install64.bat` (as Administrator)
- Script expects to run from E:\ drive (edit for your setup)

**Installing:**
```bash
# From Cygwin terminal:
./build.sh build install  # Will prompt for Admin terminal
```

**Install Location:** `C:\Program Files\Sunder.Net\LisaEm\`

### Linux/GTK

**Requirements:**
- GTK 3 development libraries
- SDL2 (for sound)
- pulseaudio or ALSA proxy

**Building:**
```bash
cd scripts
./build-wx3.1.5-gtk.sh
export PATH=/usr/local/wx3.1.5-gtk3-x64/bin:$PATH
cd ..
./build.sh clean build
sudo ./build.sh install
```

## Code Style & Conventions

**Naming:**
- `lisa_*` prefix for emulation core functions
- Hardware components named after chips (via6522, z8530, etc.)
- Global state in `vars.h`
- Function prototypes in corresponding `.h` files

**Memory Access:**
- Always use `lisa_rb()`, `lisa_rw()`, `lisa_rl()` for reads
- Always use `lisa_wb()`, `lisa_ww()`, `lisa_wl()` for writes
- Never access memory directly (goes through MMU)

**Timing:**
- All operations measured in 68000 CPU cycles
- Use `cpu68k_clocks` for current cycle count
- Event queue (`fliflo_queue`) for delayed events

**Build System:**
- Each subdirectory with code has its own `build.sh`
- Sub-builds are sourced, not executed (must not use `exit` except on error)
- Use `progressbar()` function for status updates
- Use `qjob()` for parallel compilation

## Important Global Variables

From `src/include/vars.h`:

```c
// Display
int lisa_vid_size_x = 720;        // Screen width
int lisa_vid_size_y = 364;        // Screen height
uint32 videolatch;                // Video control latch

// CPU & Memory
uint32 cpu68k_clocks;             // Current CPU cycle count
uint32 segment1, segment2;        // MMU segments
uint32 context, lastcontext;      // MMU context (0-7)

// Devices
uint8 via_running;                // VIA timer active
int16 copsqueuelen;               // COPS event queue length
uint8 floppy_irq_top;             // Floppy interrupt

// Mouse/Input
int16 last_mouse_x, last_mouse_y; // Mouse coordinates
int16 mousequeuelen;              // Mouse event queue

// Timing
MC68K_CLOCKS z8530_event;         // Next Z8530 event
MC68K_CLOCKS cops_event;          // Next COPS event
MC68K_CLOCKS tenth_sec_cycles;    // Cycles per 1/10 second
```

## External Resources

- Homepage: https://lisaem.sunder.net/
- Lisa FAQ: https://lisafaq.sunder.net
- Community: https://lisalist2.com
- GitHub: https://github.com/rayarachelian/lisaem
- BashBuild: https://github.com/rayarachelian/bashbuild

## Command Line Reference

```bash
# Build commands
./build.sh clean              # Clean build artifacts
./build.sh build              # Build only
./build.sh clean build        # Clean and build
./build.sh install            # Build and install
./build.sh package            # Create OS packages

# Build options
--with-debug                  # Include debug symbols
--with-tracelog               # Enable trace logging
--64                          # Force 64-bit build
-arch=ppc                     # Cross-compile for PPC

# Running
lisaem                        # Launch emulator
lisafsh-tool image.dc42       # Interactive disk shell
lisadiskinfo image.dc42       # Display disk info
```
