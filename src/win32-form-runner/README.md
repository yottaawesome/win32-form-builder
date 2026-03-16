# Win32 Form Runner

A lightweight runtime that loads a JSON form definition and displays it as a native Win32 window. Use this to preview and test forms created with the [form designer](../win32-form-designer/README.md).

## Usage

```
win32-form-runner.exe [path-to-form.json]
```

- With a path argument: loads that JSON file
- With no arguments: loads `sample-form.json` from the working directory

## How It Works

The runner is a thin host (~50 lines) that:

1. Initializes Common Controls and RichEdit
2. Loads the JSON form via `FormDesigner::LoadFormFromFile()`
3. Wires up event handlers programmatically via `FormDesigner::EventMap`
4. Creates the window with `FormDesigner::LoadForm()`
5. Runs the Win32 message loop

### Example: Adding Event Handlers

```cpp
auto events = FormDesigner::EventMap{};

events.onClick(301, [](const FormDesigner::ClickEvent& e) {
    Win32::MessageBoxW(e.formHwnd, L"Submitted!", L"OK", Win32::Mb_Ok);
});

events.onClick(302, [](const FormDesigner::ClickEvent& e) {
    Win32::DestroyWindow(e.formHwnd);
});

auto hwnd = FormDesigner::LoadForm(form, hInstance, events);
```

Event handlers are registered by control ID. The `sample-form.json` ships with two buttons wired up: **Submit** (ID 301) shows a message box and **Cancel** (ID 302) closes the window.

## Sample Form

The included `sample-form.json` demonstrates:
- Labels and TextBoxes for Name and Email fields
- A GroupBox containing CheckBoxes
- Submit and Cancel buttons with click event handlers
- Background color, text alignment, and custom styles

## Building

### Prerequisites
- **Visual Studio 2022** (v17.x) or later with the **Desktop development with C++** workload
- The **formbuilder** library (built automatically as a project dependency)

### Build from Visual Studio
1. Open `src\win32-form-builder.sln`
2. Set **win32-form-runner** as the startup project
3. Build (Ctrl+Shift+B)

### Build from command line
```
MSBuild src\win32-form-builder.sln /p:Configuration=Debug /p:Platform=x64 /m
```

Output: `src\x64\Debug\win32-form-runner.exe`

### Debugging
Set **Command Arguments** to the path of a JSON file in project properties → Debugging. Press F5.

## Dependencies

- **formbuilder** static library (project reference)
- `Comctl32.lib` (Windows Common Controls)
- Common Controls v6 manifest (for visual styles)
