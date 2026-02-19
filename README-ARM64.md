# LisaEm (Experimental Apple Silicon Derivative)

**Version:** 1.2.9-EXPERIMENTAL-ARM64  
**Target OS:** macOS 26.2+ (Apple Silicon / arm64)  
**Status:** Experimental / Derivative Work

## Overview

This is a derivative work of LisaEm, the Apple Lisa emulator originally created by **Ray Arachelian**.

This specific fork (`sgtbilge/lisaem`) is an **experimental project** specifically modified and optimized for modern macOS versions (26.2 and greater) running on **Apple Silicon (M1/M2/M3/M4)**.

## Key Changes in this Derivative

- **Native Apple Silicon Support:** Build system updated to target `arm64` natively.
- **Modern SDK Integration:** Fixed build process to work with modern Xcode/macOS SDKs using `xcrun`.
- **Improved Stability:** 
  - Fixed a crash on exit related to terminal cleanup.
  - Added a graceful Quit confirmation dialog that offers to trigger the virtual Lisa's power button for a safe guest OS shutdown.
- **Data Robustness:** Enabled `synconwrite` for ProFile hard drive images to reduce the risk of corruption during improper host-side shutdowns.
- **Improved Error Handling:** CPU fatal errors ("Odd PC") now offer a "Reboot" option instead of an immediate crash.

## Original Credits

- **Ray Arachelian:** Original creator and lead developer of LisaEm.
- **James Ponder:** 68K CPU core.
- **Maxim Stepin, Cameron Zemek, Francois Gannaz:** hqx scaling algorithms.

## Repository & Attribution

- **Original Project:** [https://github.com/rayarachelian/lisaem](https://github.com/rayarachelian/lisaem)
- **This Fork:** [https://github.com/sgtbilge/lisaem](https://github.com/sgtbilge/lisaem)

This derivative is released under the same license as the original (GNU GPL v3).
Modified in 2026 by Gemini CLI for sgtbilge.
