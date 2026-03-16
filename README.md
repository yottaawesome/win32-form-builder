# win32-form-builder

A visual form designer and runtime for Win32 applications, built with C++20 modules.

Design forms by dragging and dropping controls onto a canvas, configure properties and events, then save as JSON or export as standalone C++ source code. Load and run the forms with the included runtime.

## Projects

| Project | Type | Description |
|---------|------|-------------|
| [formbuilder](src/formbuilder/README.md) | Static library | Core data model, JSON parser/serializer, runtime loader, C++ code generator |
| [win32-form-designer](src/win32-form-designer/README.md) | Application | Visual drag-and-drop form designer with toolbox, property editor, and canvas |
| [win32-form-runner](src/win32-form-runner/README.md) | Application | Lightweight runtime that loads and displays JSON forms |
| [tests](src/tests/README.md) | Console app | 99 unit tests (385 assertions) using Catch2 |

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

- **16 control types**: Button, CheckBox, RadioButton, Label, TextBox, GroupBox, ListBox, ComboBox, ProgressBar, TrackBar, DateTimePicker, TabControl, ListView, TreeView, UpDown, RichEdit
- **Visual design**: Drag-and-drop placement, resize handles, snap guides, ruler guides, grid overlay
- **Property editing**: Type, text, ID, position, size, alignment, events, styles, locking
- **Undo/Redo**, Cut/Copy/Paste/Duplicate, z-order management
- **Export to C++**: Generate standalone Win32 apps (classic or C++20 module style)
- **Live preview** (F5) within the designer
- **JSON format** for form definitions

## Technology

- **C++20 modules** (`.ixx` files) — no traditional headers
- **Win32 API** — pure native Windows, no frameworks
- **nlohmann/json** for JSON parsing (via vcpkg)
- **Catch2 v3** for testing (via vcpkg)

## License

See [LICENSE](LICENSE).