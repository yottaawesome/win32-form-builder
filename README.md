# Win32 Form Builder

## Introduction

An experimental visual form designer and runtime for Win32 applications, built with C++20 modules, that uses JSON as its layout format. Design forms by dragging and dropping controls onto a canvas, configure properties and events, then save as JSON or export as standalone C++ source code. Load and run the forms with the included runtime. This project has been almost exclusively vibe coded as an experiment in AI-first development workflows.

## Why a form builder?

Native Win32 form builders are sorely lacking, and the idea has been in the back of my head for some time to experiment with building my own. Currently, robust form designers really only exist for the dotnet ecosystem (e.g. WPF), with the native C++ Win32 space largely dependent on designers provided by frameworks like Qt or wxWidgets. While these frameworks are powerful, they're built for cross-platform development and are often simply overengineered for building simple Win32 forms. Many of these frameworks are also quite old, having their use rooted in the design choices of '90s and 2000s-style object-oriented programming, whereas I was wanting to experiment with newer features of C++. 

On the Microsoft front for UI frameworks, things are even worse. Despite being billed as the premier UI framework for Windows, WinUI3 still lacks a visual designer years after its release. Moreover, Microsoft seems to be living up to its track record of creating UI frameworks only to slowly abandon them years later, with WinUI3 seeing significantly less investment from Microsoft as time has gone on. This is not even mentioning that building WinUI3 UIs with C++ is a hot mess of IDL and COM, with the end result being a UI that consumes Electron-like amounts of system resources for even the most basic examples.

## AI disclosure

This project has been pretty much written by Copilot with Claude Opus 4.6, with review and feature suggestions from me.

## Projects

| Project | Type | Description |
|---------|------|-------------|
| [formbuilder](src/formbuilder/README.md) | Static library | Core data model, JSON parser/serializer, runtime loader, C++ code generator |
| [win32-form-designer](src/win32-form-designer/README.md) | Application | Visual drag-and-drop form designer with toolbox, property editor, and canvas |
| [win32-form-runner](src/win32-form-runner/README.md) | Application | Lightweight runtime that loads and displays JSON forms |
| [tests](src/tests/README.md) | Console app | 122 unit tests (437 assertions) using Catch2 |

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
- **Property editing**: Type, text, ID, position, size, alignment, anchoring, events, styles, locking
- **Property validation**: Invalid values are highlighted with visual feedback and rejected on focus loss
- **Control anchoring**: Configure how controls resize/reposition when the form is resized (Top, Bottom, Left, Right flags)
- **Control grouping**: Group related controls (Ctrl+G) so they select and move together
- **Tab order editor**: Visual click-to-assign tab index mode with numbered badges
- **Dark mode**: Toggle between light and dark themes (persisted to settings)
- **Recent files**: Quick access to recently opened/saved forms in the File menu
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
