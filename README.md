# Win32 Form Builder

## Introduction

An experimental visual form designer and runtime for Win32 applications, built with C++20 modules, that uses JSON as its layout format. Design forms by dragging and dropping controls onto a canvas, configure properties and events, then save as JSON or export as standalone C++ source code or `.rc` dialog resources. Load and run the forms with the included runtime. This project has been almost exclusively vibe coded as an experiment in AI-first development workflows.

## Why a form builder?

Native Win32 form builders are sorely lacking, and the idea has been in the back of my head for some time to experiment with building my own. Currently, robust form designers really only exist for the dotnet ecosystem (e.g. WPF), with the native C++ Win32 space largely dependent on designers provided by frameworks like Qt or wxWidgets. While these frameworks are powerful, they're built for cross-platform development and are often simply overengineered for building simple Win32 forms. Many of these frameworks are also quite old, having their use rooted in the design choices of '90s and 2000s-style object-oriented programming, whereas I was wanting to experiment with newer features of C++. 

On the Microsoft front for UI frameworks, things are even worse. Despite being billed as the premier UI framework for Windows, WinUI3 still lacks a visual designer years after its release. Moreover, Microsoft seems to be living up to its track record of creating UI frameworks only to slowly abandon them years later, with WinUI3 seeing significantly less investment from Microsoft as time has gone on. This is not even mentioning that building WinUI3 UIs with C++ is a hot mess of IDL and COM, with the end result being a UI that consumes Electron-like amounts of system resources for even the most basic examples.

## AI disclosure

This project has been pretty much written by Copilot with Claude Opus 4.6, with review and feature suggestions from me.

## Projects

| Project | Type | Description |
|---------|------|-------------|
| [formbuilder](src/formbuilder/README.md) | Static library | Core data model, JSON parser/serializer, runtime loader, C++ code generator, RC export, accessibility audit, typed control wrappers |
| [win32-form-designer](src/win32-form-designer/README.md) | Application | Visual drag-and-drop form designer with toolbox, property editor, alignment tools, templates, and canvas |
| [win32-form-runner](src/win32-form-runner/README.md) | Application | Lightweight runtime that loads and displays JSON forms with DPI awareness and hot reload |
| [tests](src/tests/README.md) | Console app | 467 test cases (1183 assertions) using Catch2 |

## Quick Start

### Prerequisites
- **Visual Studio 2022** (v17.x) or later with the **Desktop development with C++** workload
- **vcpkg** (integrated with Visual Studio)

### Build
```
MSBuild src\win32-form-builder.sln /p:Configuration=Debug /p:Platform=x64 /m
```

### Run the Designer
```
src\x64\Debug\win32-form-designer.exe
```

### Run a Form
```
src\x64\Debug\win32-form-runner.exe src\win32-form-runner\sample-form.json
```

### Run Tests
```
src\x64\Debug\tests.exe
```

## Features

### Controls
- **23 control types**: Button, CheckBox, RadioButton, Label, TextBox, GroupBox, ListBox, ComboBox, ProgressBar, TrackBar, DateTimePicker, TabControl, ListView, TreeView, UpDown, RichEdit, MonthCalendar, Link, IPAddress, HotKey, Picture, Separator, Animation

### Visual Design
- Drag-and-drop placement with resize handles and snap-to-align guides
- Grid overlay with configurable snap, ruler guides, form boundary
- Keyboard nudge (arrow keys), multi-select with Ctrl+Click
- **Alignment toolbar**: 6 align operations, 2 distribute, 3 match-size (Format menu, toolbar, context menu)
- **Per-Monitor V2 DPI awareness**: scales UI and canvas correctly on high-DPI displays

### Properties
- Type, text, ID, position, size, alignment, anchoring, events, styles, locking
- **Font properties**: per-control font family, size, bold, italic with inherited defaults
- **Per-control tooltips**: tooltip text shown at runtime on hover
- **Visible/Enabled**: per-control visibility and enabled/disabled state with designer overlays (hatching)
- **Value, Min, Max**: initial value and range for ProgressBar, TrackBar, UpDown
- **ComboBox/ListBox items**: edit items and selected index directly in the designer
- **Picture image path**: browse for BMP/ICO images with runtime loading
- **Data binding**: bind struct name on Form, bind field on each Control, with PopulateForm/ReadForm codegen
- **Validation metadata**: required, min/max length, pattern, min/max range per control type
- **Accessibility**: tab stop, group start, accessible name/description, with 7-rule audit (Tools menu)
- Property validation with visual error highlighting

### Editing
- Undo/Redo, Cut/Copy/Paste/Duplicate, Select All, Delete
- Control grouping (Ctrl+G) and locking
- Tab order editor with visual click-to-assign badges
- Z-Order panel with multi-select, inline rename, delete, keyboard shortcuts

### Typed Control Wrappers
- **Zero-overhead** non-owning HWND wrappers for improved developer ergonomics
- **`FormWindow`**: wraps the form HWND with `Get<T>(id)` template access to child controls
- **12 typed wrappers**: Button, TextBox, CheckBox, RadioButton, Label, ComboBox, ListBox, ProgressBar, TrackBar, UpDown, DateTimePicker, RichEdit — each with type-specific methods
- **`ControlBase`**: common API for all controls — GetText, SetText, Show, Hide, Enable, Disable, Focus, IsVisible, IsEnabled
- **Wrapper-based event binding**: `window.GetButton(id).OnClick(handler)` — bind events directly on typed wrappers after `LoadForm()`

### Runtime
- **Modal dialogs**: `ShowModalForm()` / `EndModal()` for modal form workflows returning `DialogResult`
- **Message box helpers**: `ShowInfo`, `ShowError`, `ShowWarning`, `AskYesNo`, `AskYesNoCancel`, `AskOkCancel` — simplified wrappers around `MessageBoxW`
- **Hot reload**: `EnableHotReload()` watches the JSON file and automatically rebuilds the form when saved — ideal for rapid iteration
- **Generated control IDs**: export named `constexpr` ID constants from the designer (File → Export Control IDs)

### Error Handling
- **`FormException`**: structured exception class deriving from `std::exception` with `FormErrorCode` enum
- **`std::expected` overloads**: `TryLoadForm`, `TryLoadFormFromFile`, `TryShowModalForm` return errors without throwing

### Export & File
- **Export to C++**: standalone Win32 apps (classic or C++20 module style) with event stubs, data binding helpers, and DPI scaling
- **Export to .rc**: Win32 dialog resource files with pixel-to-DLU conversion and `resource.h` header
- **Built-in templates**: Login, Settings, Data Entry, About, Search starter layouts
- Recent files, JSON format, live preview (F5)
- **Dark mode** with settings persistence (`designer.ini`)

## Technology

- **C++20 modules** (`.ixx` files) — no traditional headers
- **Win32 API** — pure native Windows, no frameworks
- **nlohmann/json** for JSON parsing (via vcpkg)
- **Catch2 v3** for testing (via vcpkg)

## License

See [LICENSE](LICENSE).
