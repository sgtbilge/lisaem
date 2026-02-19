# Project Context: macOS 26.3 (Tahoe) Development
- **Language**: Swift 6.3 (Strict Concurrency enabled).
- **Frameworks**: SwiftUI 7.0, AppKit 26.3.
- **Rules**: 
  - Use the new `@Observable` macros only.
  - Prioritize `Async/Await` over Combine.
  - Assume the target is macOS 26.3+.