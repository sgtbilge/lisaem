## 🛠️ wxWidgets Developer Skills Checklist (`skills.md`)

### 1. Fundamentals & Architecture

* [ ] **Event Handling:** Mastered the `Bind()` syntax (modern) vs. Event Tables (legacy).
* [ ] **Window Hierarchy:** Understanding the parent-child relationship and how it affects memory management (automatic deletion).
* [ ] **Top-Level Windows:** Proficiency with `wxFrame` and `wxDialog` lifecycles.
* [ ] **Cross-Platform Entry:** Implementing `wxApp` and `wxIMPLEMENT_APP`.

### 2. Layout Management (The "Sizer" Logic)

* [ ] **Box Sizers:** Proficiency in `wxBoxSizer` (Horizontal/Vertical).
* [ ] **Grid Sizers:** Using `wxGridSizer` and `wxFlexGridSizer` for complex forms.
* [ ] **Sizer Flags:** Using `wxSizerFlags` for cleaner, more readable layout code.
* [ ] **Dynamic Layout:** Implementing `Fit()`, `Layout()`, and handling `wxSizeEvent`.

### 3. Advanced UI Components

* [ ] **Complex Controls:** Mastering `wxDataViewCtrl` or `wxGrid` for data-heavy apps.
* [ ] **AUI (Advanced User Interface):** Implementing docking panes and floating toolbars with `wxAuiManager`.
* [ ] **WebView:** Integrating browser engines via `wxWebView`.
* [ ] **Graphics Context:** Using `wxGraphicsContext` or `wxDC` for custom 2D drawing and high-DPI support.

### 4. System & Platform Integration

* [ ] **MacOS Tahoe Specifics:** Handling "Liquid Glass" rendering and native fullscreen Spaces via `ShowFullScreen`.
* [ ] **Dark Mode:** Implementing `wxSystemSettings` listeners to adapt UI colors dynamically.
* [ ] **High DPI:** Handling `FromDIP()` and `GetContentScaleFactor()` for 4K/Retina displays.
* [ ] **Multithreading:** Using `wxThread` and safely updating the UI from worker threads via `wxQueueEvent`.

### 5. Build Systems & Tooling

* [ ] **CMake:** Writing `CMakeLists.txt` that correctly find and link `wxWidgets`.
* [ ] **Resources:** Handling `.rc` files (Windows) and `.plist`/Bundle structures (macOS).
* [ ] **Localization:** Using `wxLocale` and `gettext` (.po/.mo files) for internationalization.

---

### Pro-Tips for your 2026 Workflow:

* **Avoid `wxT()` and `_T()`:** These are considered obsolete in modern C++. Just use standard string literals or `_("string")` for translation.
* **Smart Pointers:** While wxWidgets manages its own window memory, use `std::unique_ptr` for non-window objects to avoid the "manual delete" pitfalls of 1990s-era C++.
