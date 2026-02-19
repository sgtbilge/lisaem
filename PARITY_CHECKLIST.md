# LisaEm Swift Parity Checklist

Goal: match original wx startup/runtime behavior before making further functional changes.

Status legend:
- `PASS`: behavior appears equivalent.
- `PARTIAL`: some parity implemented, but gaps remain.
- `MISMATCH`: clear behavior difference.

## 1) Initialization Order

Checks:
- Core init order (SCC disconnect/init -> CPU init -> MMU -> IPC allocator -> ROM load -> video init -> cpu reset)
- Device init order (COPS/IRQ/VIA/Profile/Floppy)
- Serial setup timing
- Keyboard ID + serial application timing

Findings:
- `PARTIAL`: high-level order is similar, but not identical, and some steps happen in different phases.
- Original order reference: `src/host/wxui/lisaem_wx.cpp:7418`, `src/host/wxui/lisaem_wx.cpp:7421`, `src/host/wxui/lisaem_wx.cpp:7434`, `src/host/wxui/lisaem_wx.cpp:7437`, `src/host/wxui/lisaem_wx.cpp:7440`, `src/host/wxui/lisaem_wx.cpp:7471`, `src/host/wxui/lisaem_wx.cpp:7592`
- Swift headless order reference: `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:257`, `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:260`, `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:261`, `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:264`, `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:305`, `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:311`

Notes:
- Original applies keyboard ID after RAM allocation and before serial parse path; headless applies earlier.
- Original runs more UI-side setup between ROM load and reset (display/device wiring).
- Swift headless now applies `fixromchk()` after successful ROM load to match wx ROM-init behavior.

## 2) Event Loop / Timing Model

Checks:
- Host timer cadence and CPU execution scheduling
- Throttle/emulation parameters and runtime clock reset behavior
- Timer event dispatch cadence (COPS/decisecond/seek_mouse)

Findings:
- `PARTIAL`: major gaps reduced, but runtime pacing still needs validation against wx traces.
- Original uses wx timer + runtime/throttle clock management: `src/host/wxui/lisaem_wx.cpp:6680`, `src/host/wxui/lisaem_wx.cpp:5706`, `src/host/wxui/lisaem_wx.cpp:5715`
- Swift now uses `emulationTick`-driven host timer cadence and `emulation_time`-scaled cycle budget: `lisaem-swift/Sources/LisaEm/EmulatorEngine.swift:65`, `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:321`

Notes:
- This is one of the highest-risk parity gaps for ROM diagnostics loops.

## 3) Config Parity

Checks:
- ROM/dual ROM, parallel path/device, memory, HLE, cheat ROM tests, I/O ROM, keyboard ID, serial number
- Default handling when config keys are missing
- Additional wx config domains (PRAM/clock/serial slots)

Findings:
- `PARTIAL`: key boot fields are parsed and PRAM/clock handoff is wired, but full wx config parity is not complete.
- Swift parsed fields: `lisaem-swift/Sources/LisaEm/AppModel.swift:180` through `lisaem-swift/Sources/LisaEm/AppModel.swift:217`
- Swift now also forwards `[pram]` rows and `[clock]` values into core before init.
- Original parsed fields include broader state (PRAM/clock/serial/cardslots): `src/host/wxui/LisaConfig.cpp:113` through `src/host/wxui/LisaConfig.cpp:190`

Notes:
- Slot/serial/cardslot behavior remains incomplete; serial loopback config and slot path parsing are now partially implemented, but full device wiring parity is still pending.
- PRAM/clock parity application is now done after core initialization in Swift app startup so `unvars()` does not wipe forwarded config state before first run.

## 4) Device Attachment / Media Mapping

Checks:
- Built-in parallel device attach path semantics (`/parallelport/path`)
- ProFile vs floppy mapping
- Mount fallback behavior

Findings:
- `PARTIAL`: Swift no longer does basename fallback search; built-in parallel profile wiring now occurs during core init with configured path/device.
- Swift profile mount path behavior: `lisaem-swift/Sources/LisaEm/AppModel.swift:109` through `lisaem-swift/Sources/LisaEm/AppModel.swift:123`
- Original uses configured path/device wiring via `connect_device_to_via(...)`: `src/host/wxui/lisaem_wx.cpp:7542`

Notes:
- Previous fallback-induced mis-mount risk removed; still need full card-slot/wiring parity.

## 5) Glue / Stub Behavioral Parity

Checks:
- `messagebox`, `pickprofilesize`, reboot/power hooks, serial connect hooks
- Behavior under ROM errors and media prompts

Findings:
- `PARTIAL`: headless stubs still shortcut UI control flow, but default yes/no behavior now allows non-interactive continuation.
- Stub references: `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:215` through `lisaem-swift/Sources/LegacyCore/lisaem_headless.cpp:223`

Notes:
- Returning fixed values (`pickprofilesize=0`, `yesnomessagebox=0`) changes behavior under missing/corrupt media paths.

## 6) Wrapper-Introduced Logic Changes

Checks:
- Any non-upstream logic in Swift/headless wrapper touching boot behavior
- COPS/IRQ signaling policy differences

Findings:
- `PARTIAL`: prior troubleshooting deltas were removed; wrapper still differs in non-UI flow and stubs.
- Boot-speed clamp removed; COPS/VIA IRQ paths restored to source parity.

Notes:
- Any wrapper-side mitigation can help debugging, but is a parity risk until baseline is proven.

## Priority Order (No New Behavior Changes Yet)

1. `DONE` Lock media mapping parity (disable aggressive fallback profile mounting).
2. `DONE` Freeze timing model parity baseline (remove dynamic boot clamp / match cadence knobs).
3. `DONE` Re-test with minimal logging and compare boot PC/IFR milestones.
4. `IN PROGRESS` Freeze remaining init/config parity (slot/serial/cardslot wiring and full wx config semantics).
5. `NEXT` Apply targeted bug fixes only after remaining parity gaps are closed.
