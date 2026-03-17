# Formbuilder Library

A C++20 static library for loading, saving, code-generating, and validating Win32 forms defined in JSON.

## What It Does

The formbuilder library provides the core data model and operations used by both the **form runner** and the **form designer**:

- **Parse** JSON form definitions into structured C++ objects
- **Serialize** form objects back to JSON
- **Load** forms at runtime — creates real Win32 windows and controls from JSON with DPI scaling
- **Generate C++ code** — produces standalone `.cpp` files from form definitions with event stubs, data binding helpers, and DPI-aware layout
- **Generate .rc dialog resources** — exports Win32 dialog templates with pixel-to-DLU conversion and `resource.h` header
- **Accessibility audit** — checks forms for 7 common accessibility issues (missing labels, access keys, tab stops, etc.)
- **Event routing** — type-safe event dispatch (click, change, focus, blur, check, double-click, selection change)
- **Typed control wrappers** — zero-overhead `FormWindow` and 12 typed HWND wrappers (Button, TextBox, CheckBox, etc.) for ergonomic control access after `LoadForm()`, with wrapper-based event binding
- **Modal dialogs** — `ShowModalForm()` / `EndModal()` for blocking dialog workflows returning `DialogResult`
- **Message box helpers** — `ShowInfo`, `ShowError`, `ShowWarning`, `AskYesNo`, `AskYesNoCancel`, `AskOkCancel`
- **Hot reload** — `EnableHotReload()` watches JSON files and automatically rebuilds the form when saved
- **Error handling** — `FormException` with error codes for throwing APIs, `std::expected` for nothrow variants (`TryLoadForm`, `TryLoadFormFromFile`, `TryShowModalForm`)

## Module Partitions

The library is a single C++20 module (`formbuilder`) split into partitions:

| Partition | File | Purpose |
|-----------|------|---------|
| `:win32` | `win32.ixx` | Win32 API imports — the only file that includes `<Windows.h>` |
| `:json` | `json.ixx` | Re-exports `nlohmann::json` from the global module fragment |
| `:schema` | `schema.ixx` | Core types: `ControlType` (23 types), `TextAlign`, `Anchor`, `Control`, `Form`, `Rect`, `FontInfo`, `ValidationInfo`, `DesignerGuide` |
| `:events` | `events.ixx` | Event structs (`ClickEvent`, `ChangeEvent`, etc.) and `EventMap` dispatcher |
| `:errors` | `errors.ixx` | `FormErrorCode` enum and `FormException` exception class |
| `:parser` | `parser.ixx` | `ParseForm(json)` — JSON → `Form` |
| `:serializer` | `serializer.ixx` | `SerializeForm(Form)` → JSON string |
| `:codegen` | `codegen.ixx` | `GenerateCode(Form, useModules)` → standalone C++ source; `GenerateRcDialog(Form)` and `GenerateRcHeader(Form)` → `.rc` / `.h` files |
| `:loader` | `loader.ixx` | `LoadForm()`, `LoadFormFromFile()` — creates Win32 windows at runtime with anchor-based resize, DPI scaling, range/value initialization |
| `:accessibility` | `accessibility.ixx` | `CheckAccessibility(Form)` — 7-rule audit returning warnings/errors |
| `:controls` | `controls.ixx` | `ControlBase`, 12 typed wrappers (`Button`, `TextBox`, `CheckBox`, etc.), `FormWindow` — non-owning HWND wrappers with event binding |
| `:messagebox` | `messagebox.ixx` | `ShowInfo`, `ShowError`, `ShowWarning`, `AskYesNo`, `AskYesNoCancel`, `AskOkCancel` — message box helpers |

The facade (`formbuilder.ixx`) re-exports all partitions.

## Supported Control Types

Button, CheckBox, RadioButton, Label, TextBox, GroupBox, ListBox, ComboBox, ProgressBar, TrackBar, DateTimePicker, TabControl, ListView, TreeView, UpDown, RichEdit, MonthCalendar, Link, IPAddress, HotKey, Picture, Separator, Animation

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

### Typed Control Wrappers

After `LoadForm()`, wrap the returned HWND in a `FormWindow` to access controls with type-safe wrappers instead of raw Win32 API calls:

```cpp
auto window = FormDesigner::FormWindow{hwnd};

// Get typed wrappers by control ID
auto nameBox  = window.GetTextBox(101);
auto okButton = window.GetButton(102);
auto agree    = window.GetCheckBox(103);
auto items    = window.GetComboBox(104);

// Type-specific methods
nameBox.SetText(L"Hello");
auto name = nameBox.GetText();       // std::wstring
auto len  = nameBox.GetTextLength(); // int

agree.Toggle();
if (agree.IsChecked()) { /* ... */ }

items.AddItem(L"Option A");
items.SetSelectedIndex(0);
auto count = items.GetCount();

// Common methods on all controls (via ControlBase)
okButton.Enable();
okButton.Disable();
okButton.Show();
okButton.Hide();
okButton.Focus();
auto visible = okButton.IsVisible();  // bool

// Form-level operations
window.SetTitle(L"New Title");
window.Close();
```

All wrappers are **non-owning** (just an HWND, freely copyable, zero overhead). The `Get<T>(id)` template is constrained to types derived from `ControlBase`:

```cpp
auto ctrl = window.Get<FormDesigner::ProgressBar>(105);
ctrl.SetRange(0, 100);
ctrl.SetValue(42);
```

Available wrappers: `Button`, `Label`, `TextBox`, `RichEdit`, `CheckBox`, `RadioButton`, `ComboBox`, `ListBox`, `ProgressBar`, `TrackBar`, `UpDown`, `DateTimePicker`.

### Wrapper-Based Event Binding

Typed wrappers support direct event binding after `LoadForm()`:

```cpp
auto hwnd = FormDesigner::LoadForm(form, hInstance, events);
auto window = FormDesigner::FormWindow{hwnd};

// Bind events directly on wrappers — cleaner than EventMap for post-load wiring.
window.GetButton(Controls::CancelButton).OnClick([](const FormDesigner::ClickEvent& e) {
    Win32::DestroyWindow(e.formHwnd);
});

window.GetTextBox(Controls::NameText).OnChange([](const FormDesigner::ChangeEvent& e) {
    /* text changed */
});

window.GetCheckBox(Controls::AgreeCheck).OnCheck([](const FormDesigner::CheckEvent& e) {
    /* check toggled */
});
```

Each wrapper exposes only the events that make sense for its control type (e.g. Button has `OnClick`/`OnDoubleClick`/`OnFocus`/`OnBlur`, TextBox has `OnChange`/`OnFocus`/`OnBlur`, CheckBox has `OnClick`/`OnCheck`/`OnFocus`/`OnBlur`).

### Modal Dialogs

```cpp
auto dialogForm = FormDesigner::LoadFormFromFile("confirm.json");
auto dlgEvents = FormDesigner::EventMap{};
dlgEvents.onClick(1, [](const FormDesigner::ClickEvent& e) {
    FormDesigner::EndModal(e.formHwnd, FormDesigner::DialogResult::Ok);
});

auto result = FormDesigner::ShowModalForm(dialogForm, hInstance, dlgEvents, parentHwnd);
if (result == FormDesigner::DialogResult::Ok) { /* user confirmed */ }
```

### Message Box Helpers

```cpp
// Notification dialogs
FormDesigner::ShowInfo(hwnd, L"Operation completed.", L"Success");
FormDesigner::ShowError(hwnd, L"File not found.", L"Error");
FormDesigner::ShowWarning(hwnd, L"Unsaved changes.", L"Warning");

// Prompt dialogs returning DialogResult
auto result = FormDesigner::AskYesNo(hwnd, L"Save changes?", L"Confirm");
if (result == FormDesigner::DialogResult::Yes) { /* save */ }

auto r2 = FormDesigner::AskOkCancel(hwnd, L"Proceed?", L"Confirm");
auto r3 = FormDesigner::AskYesNoCancel(hwnd, L"Save before closing?", L"Confirm");
```

### Hot Reload

```cpp
auto hwnd = FormDesigner::LoadForm(form, hInstance, events, basePath);
FormDesigner::EnableHotReload(hwnd, "my-form.json", basePath);
// The form automatically rebuilds when the JSON file is saved.
// Call DisableHotReload(hwnd) to stop watching.
```

### Error Handling

Throwing APIs raise `FormException` with a `FormErrorCode`:

```cpp
try {
    auto form = FormDesigner::LoadFormFromFile("missing.json");
} catch (const FormDesigner::FormException& ex) {
    ex.code();  // FormErrorCode::FileNotFound, ParseError, etc.
    ex.what();  // human-readable message
}
```

Nothrow alternatives return `std::expected`:

```cpp
auto result = FormDesigner::TryLoadFormFromFile("form.json");
if (result) {
    auto& form = result.value();
} else {
    auto& err = result.error(); // FormException
}
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

auto rc = FormDesigner::GenerateRcDialog(form);
auto header = FormDesigner::GenerateRcHeader(form);
// Returns .rc dialog resource and resource.h content
```

### Accessibility Audit
```cpp
auto warnings = FormDesigner::CheckAccessibility(form);
for (auto& w : warnings)
    // w.level (Warning/Error), w.controlIndex, w.message
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
  "bindStruct": "FormData",
  "controls": [
    {
      "type": "Button",
      "text": "&OK",
      "id": 101,
      "rect": [10, 20, 80, 25],
      "style": 1,
      "textAlign": "center",
      "onClick": "handle_ok",
      "tooltip": "Click to confirm",
      "tabStop": true,
      "accessibleName": "OK button"
    }
  ],
  "guides": [
    { "horizontal": true, "position": 100 }
  ]
}
```

### Control Properties
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `type` | string | | One of the 23 supported control types |
| `text` | string | `""` | Control text / label |
| `id` | int | `0` | Control identifier for events and `GetDlgItem` |
| `rect` | `[x, y, w, h]` | | Position and size in pixels |
| `style` | int | `0` | Additional Win32 window style flags |
| `textAlign` | string | `"left"` | `"left"`, `"center"`, or `"right"` |
| `tabIndex` | int | `0` | Tab order |
| `anchor` | `["top","left",...]` | `["top","left"]` | Resize anchoring flags |
| `locked` | bool | `false` | Locked in the designer (not editable) |
| `visible` | bool | `true` | Whether control has WS_VISIBLE |
| `enabled` | bool | `true` | Whether control is enabled (WS_DISABLED when false) |
| `groupId` | int | `0` | Designer control group membership |
| `tooltip` | string | `""` | Tooltip text shown on hover |
| `font` | object | | `{ "family", "size", "bold", "italic" }` — overrides form font |
| `items` | string[] | `[]` | List items (ComboBox/ListBox only) |
| `selectedIndex` | int | `-1` | Pre-selected item index (ComboBox/ListBox only) |
| `value` | int | `0` | Initial value (ProgressBar, TrackBar, UpDown only) |
| `imagePath` | string | `""` | Relative path to BMP/ICO file (Picture only) |
| `bindField` | string | `""` | Data binding: struct member name |
| `tabStop` | bool | `true` | Whether control receives WS_TABSTOP (interactive controls only) |
| `groupStart` | bool | `false` | Whether control starts a new WS_GROUP |
| `accessibleName` | string | `""` | Explicit accessible name for screen readers |
| `accessibleDescription` | string | `""` | Accessible description / help text |
| `validation` | object | | `{ "required", "minLength", "maxLength", "pattern", "min", "max" }` |
| `onClick`, `onChange`, ... | string | `""` | Event handler function names |

### Form Properties
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `title` | string | `""` | Window title |
| `width` | int | `640` | Form width in pixels |
| `height` | int | `480` | Form height in pixels |
| `backgroundColor` | string | `""` | Background color as `#RRGGBB` |
| `style` | int | | Win32 window style flags |
| `bindStruct` | string | `""` | Data binding: C++ struct name for codegen |
| `font` | object | | Default form font `{ "family", "size", "bold", "italic" }` |
| `visible` | bool | `true` | Whether form is shown on load (WS_VISIBLE) |
| `enabled` | bool | `true` | Whether form is enabled (WS_DISABLED when false) |
| `guides` | array | `[]` | Designer guide lines `{ "horizontal", "position" }` |
