# LisaEm wxWidgets to SwiftUI Migration Plan

## 1. Project Structure
We will create a new Xcode project structure that separates the legacy C/C++ core from the new Swift/SwiftUI interface.

```
lisaem-swift/
├── LisaEm.xcodeproj
├── LisaEm/                  # SwiftUI App Source
│   ├── App.swift
│   ├── UI/
│   │   ├── EmulatorView.swift
│   │   ├── SettingsView.swift
│   │   └── ...
│   ├── Interop/
│   │   ├── LisaEm-Bridging-Header.h
│   │   ├── EmulatorWrapper.h      # Obj-C++ Header
│   │   └── EmulatorWrapper.mm     # Obj-C++ Implementation (Glue code)
│   └── Resources/
├── LegacyCore/              # Existing C/C++ Source (Moved/Copied)
│   ├── lisa/                # src/lisa
│   ├── include/             # src/include
│   └── lib/                 # src/lib (libGenerator)
└── ...
```

## 2. Core Separation (The "Glue" Layer)
The existing `lisaem_wx.cpp` is tightly coupled with wxWidgets. We cannot use it directly. We must extract the initialization and emulation loop logic into a new "Headless" core interface.

**Tasks:**
*   Identify the "Engine" parts of `lisaem_wx.cpp` (e.g., `initialize_all_subsystems`, `EmulateLoop`, `handle_powerbutton`).
*   Create a new C++ file `LegacyCore/lisaem_headless.cpp` (or similar) that implements these functions *without* wxWidgets dependencies.
*   Stub out or replace wxWidgets calls (e.g., `wxMessageBox`, `wxConfig`) with callbacks or abstract interfaces that Swift can provide implementation for.

## 3. Build System (Xcode)
*   **Target 1: LegacyCore (Static Lib)**
    *   Compiles `src/lisa/**/*.c`, `src/lib/libGenerator/**/*.c`.
    *   Includes the new `lisaem_headless.cpp`.
    *   **Crucial:** Header search paths must be correctly set to match the old `include` structure.
*   **Target 2: LisaEm (App)**
    *   Links `LegacyCore`.
    *   Compiles Swift UI and the Obj-C++ Wrapper.

## 4. UI Migration Strategy

| wxWidgets Component | SwiftUI Equivalent | Notes |
| :--- | :--- | :--- |
| `LisaEmFrame` | `WindowGroup` + `EmulatorView` | Main window container. |
| `LisaWin` (Canvas) | `MetalView` (via `MTKView`) | High-performance framebuffer rendering. |
| `OnPaint` | Metal Shader or `CGContext` | Blitting the `lisaram` buffer to screen. |
| `wxMenu` | `.commands { CommandMenu... }` | Native macOS menu bar. |
| `wxDialog` | `.sheet` / `Settings` Scene | Preferences and alerts. |
| `wxSound` | `AVAudioEngine` | Low-latency audio playback. |
| `wxFileConfig` | `UserDefaults` / `AppStorage` | Configuration persistence. |

## 5. Key Challenges & Solutions
*   **Framebuffer**: The core writes to `lisaram` (global). The Wrapper will expose a pointer to this memory. The SwiftUI View will create a `CGImage` or Metal Texture from this raw data every frame (60fps).
*   **Input**: `NSEvent` monitoring in the `EmulatorView` will capture key presses and mouse clicks, converting macOS keycodes to Lisa keycodes (using `LegacyCore/include/keyscan.h` mappings), passing them to the core via the Wrapper.
*   **Loop**: The emulator loop (`EmulateLoop`) needs to run on a background thread to avoid blocking the UI. `DisplayLink` or a `Task` in Swift will drive the refresh.

## 6. Next Steps
1.  Initialize the folder structure.
2.  Copy relevant source files to `LegacyCore`.
3.  Create the Xcode project.
4.  Implement the `EmulatorWrapper` to get the CPU running.
5.  Implement the Video Output.
