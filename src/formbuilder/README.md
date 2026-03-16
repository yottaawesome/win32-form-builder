# Formbuilder Library

A C++20 static library for loading, saving, and code-generating Win32 forms defined in JSON.

## What It Does

The formbuilder library provides the core data model and operations used by both the **form runner** and the **form designer**:

- **Parse** JSON form definitions into structured C++ objects
- **Serialize** form objects back to JSON
- **Load** forms at runtime — creates real Win32 windows and controls from JSON
- **Generate C++ code** — produces standalone `.cpp` files from form definitions
- **Event routing** — type-safe event dispatch (click, change, focus, blur, check, double-click, selection change)

## Module Partitions

The library is a single C++20 module (`formbuilder`) split into partitions:

| Partition | File | Purpose |
|-----------|------|---------|
| `:win32` | `win32.ixx` | Win32 API imports — the only file that includes `<Windows.h>` |
| `:json` | `json.ixx` | Re-exports `nlohmann::json` from the global module fragment |
| `:schema` | `schema.ixx` | Core types: `ControlType` (16 types), `TextAlign`, `Anchor`, `Control`, `Form`, `Rect`, `DesignerGuide` |
| `:events` | `events.ixx` | Event structs (`ClickEvent`, `ChangeEvent`, etc.) and `EventMap` dispatcher |
| `:parser` | `parser.ixx` | `ParseForm(json)` — JSON → `Form` |
| `:serializer` | `serializer.ixx` | `SerializeForm(Form)` → JSON string |
| `:codegen` | `codegen.ixx` | `GenerateCode(Form, useModules)` → standalone C++ source |
| `:loader` | `loader.ixx` | `LoadForm()`, `LoadFormFromFile()` — creates Win32 windows at runtime with anchor-based resize support |

The facade (`formbuilder.ixx`) re-exports all partitions.

## Supported Control Types

Button, CheckBox, RadioButton, Label, TextBox, GroupBox, ListBox, ComboBox, ProgressBar, TrackBar, DateTimePicker, TabControl, ListView, TreeView, UpDown, RichEdit

## API Overview

All public symbols live in the `FormDesigner` namespace. Win32 types are in the `Win32` namespace.

### Loading and Running
```cpp
import formbuilder;

auto form = FormDesigner::LoadFormFromFile("my-form.json");
auto events = FormDesigner::EventMap{};
events.onClick(101, [](const FormDesigner::ClickEvent& e) { /* ... */ });
auto hwnd = FormDesigner::LoadForm(form, hInstance, events);
FormDesigner::RunMessageLoop();
```

### Parsing and Serializing
```cpp
auto form = FormDesigner::ParseForm(nlohmann::json::parse(jsonString));
auto json = FormDesigner::SerializeForm(form);
```

### Code Generation
```cpp
auto code = FormDesigner::GenerateCode(form, /*useModules=*/true);
// Returns a complete standalone .cpp file as a string
```

## Building

### Prerequisites
- **Visual Studio 2022** (v17.x) or later with the **Desktop development with C++** workload
- **vcpkg** with `nlohmann-json` — resolved automatically via `vcpkg.json` manifest
- **Platform Toolset**: v145
- **C++ Language Standard**: C++20 (`/std:c++latest`)

### Build from command line
```
MSBuild src\formbuilder\formbuilder.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```

The output is a static library at `src\x64\Debug\formbuilder.lib`.

### Consuming the Library
Add a project reference to `formbuilder.vcxproj` and use `import formbuilder;` in your source files. The library requires linking `Comctl32.lib` in the consuming project.

## JSON Format

Forms are described as JSON objects. See `sample-form.json` in the runner project for a complete example.

```json
{
  "title": "My Form",
  "width": 640,
  "height": 480,
  "backgroundColor": "#RRGGBB",
  "style": 13565952,
  "controls": [
    {
      "type": "Button",
      "text": "OK",
      "id": 101,
      "rect": [10, 20, 80, 25],
      "style": 1,
      "textAlign": "center",
      "onClick": "handle_ok"
    }
  ],
  "guides": [
    { "horizontal": true, "position": 100 }
  ]
}
```

### Control Properties
| Field | Type | Description |
|-------|------|-------------|
| `type` | string | One of the 16 supported control types |
| `text` | string | Control text / label |
| `id` | int | Control identifier for events and `GetDlgItem` |
| `rect` | `[x, y, w, h]` | Position and size in pixels |
| `style` | int | Additional Win32 window style flags |
| `textAlign` | string | `"left"`, `"center"`, or `"right"` |
| `tabIndex` | int | Tab order |
| `anchor` | `["top","left",...]` | Resize anchoring flags (default: top+left) |
| `locked` | bool | Locked in the designer (not editable) |
| `groupId` | int | Designer control group membership |
| `onClick`, `onChange`, ... | string | Event handler function names |
