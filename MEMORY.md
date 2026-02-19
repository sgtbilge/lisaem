# LisaEm Refactoring Status: Separation of Concerns

## Objective
Decouple the Core Emulation logic from the wxWidgets User Interface to improve portability and maintainability.

## Architecture
*   **Core (`src/core/`)**:
    *   `LisaEmCore`: Encapsulates machine state, initialization, and the main emulation loop. It should rely *only* on `LisaEmUI` for interaction with the outside world.
    *   `LisaEmConfig`: A toolkit-agnostic structure for machine configuration (std::string based).
    *   `LisaEmUI`: Abstract C++ interface defining contracts for logging, status updates, display refreshing, input polling, etc.
*   **Host (`src/host/wxui/`)**:
    *   `wxLisaEmUI`: Concrete implementation of `LisaEmUI` using wxWidgets.
    *   `LisaEmFrame`: The main wxFrame window. Now owns instances of `LisaEmCore` and `wxLisaEmUI`.
    *   `lisaem_wx.cpp`: Contains the implementation of `LisaEmFrame` and legacy global glue code.

## Progress
1.  **File Creation**:
    *   Created `src/core/LisaEmCore.h`, `src/core/LisaEmCore.cpp`, `src/core/LisaEmUI.h`.
    *   Created `src/host/wxui/wxLisaEmUI.h`, `src/host/wxui/wxLisaEmUI.cpp`.
2.  **Build System**:
    *   Updated `build.sh` (PHASE2LIST) to compile the new core and bridge files.
3.  **Refactoring `lisaem_wx.cpp`**:
    *   Removed redundant `class LisaEmFrame` definition.
    *   Removed redundant `enum` ID definitions.
    *   Added `#include "lisaemframe.h"`.
    *   Added `GetEmConfig()` helper method.
    *   Updated `LisaEmFrame` constructor to initialize `m_core` and `m_ui`.
    *   Refactored `handle_powerbutton` to use `m_core->Initialize()` and `m_core->PowerOn()`.
4.  **Refactoring `lisaemframe.h`**:
    *   Added `#include` for core headers.
    *   Updated the ID `enum` with the full list of IDs from `lisaem_wx.cpp`.
    *   Added `m_core` and `m_ui` members.

## Current Blocks & Build Errors
*   **Method Mismatch**: The method declarations in `lisaemframe.h` (e.g., `OnRefresh60Hz`, `OnVideoAntiAliased`) do not match the event table entries in `lisaem_wx.cpp` (e.g., `OnRefresh60`, `OnVideoHQ35X`).
*   **Build State**: The build is currently failing due to these declaration mismatches. The last attempt to update `lisaemframe.h` via `replace` failed due to context mismatch.

## Next Actions
1.  **Fix `lisaemframe.h`**: Manually verify and rewrite the `LisaEmFrame` class declaration in `src/include/lisaemframe.h` to strictly match the method names used in the `BEGIN_EVENT_TABLE` in `lisaem_wx.cpp`.
2.  **Verify Linkage**: Ensure `lisaem_wx.cpp` compiles against the corrected header.
3.  **Logic Migration**:
    *   The `LisaEmCore::Initialize` method currently contains a copy of the logic from `initialize_all_subsystems`. We need to verify that `lisaem_wx.cpp` is calling this core method and *not* the old global function.
    *   Migrate the body of `EmulateLoop` from `lisaem_wx.cpp` into `LisaEmCore::EmulateLoop`.
4.  **Global Variable Cleanup**:
    *   Legacy C code (`src/lisa/*`) still relies on `vars.h` globals (`lisaram`, etc.). `LisaEmCore` currently sets these globals. Long term, these should be moved into the `LisaEmCore` instance, but for now, the Core acts as a manager for them.

## Important Context
*   **Legacy C/C++ Mix**: The codebase heavily mixes C and C++. `vars.h` uses macros to handle `extern` vs definition. `LisaEmCore.cpp` had to define `IN_VARS_C 0` and use `extern "C"` blocks to link correctly with the legacy backend.
*   **Symbol Conflicts**: We encountered duplicate symbol errors because `GLOBAL` macros in `vars.h` were expanding to definitions in C++ files. This was patched by ensuring `LisaEmCore.cpp` treats them as externs or by fixing `vars.h` macros (attempted, but reverted to careful usage in `.cpp`).
