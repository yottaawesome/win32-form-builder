# Win32 Form Runner

A lightweight runtime that loads a JSON form definition and displays it as a native Win32 window with Per-Monitor V2 DPI awareness. Use this to preview and test forms created with the [form designer](../win32-form-designer/README.md).

## Usage

```
win32-form-runner.exe [path-to-form.json]
```

- With a path argument: loads that JSON file
- With no arguments: loads `sample-form.json` from the working directory

## How It Works

The runner is a thin host that demonstrates:

1. Loading a JSON form via `FormDesigner::LoadFormFromFile()`
2. Using **named control ID constants** (normally generated via File → Export Control IDs)
3. Accessing controls via **typed wrappers** (`FormWindow`, `GetTextBox`, `GetCheckBox`)
4. Showing a **modal confirmation dialog** with `ShowModalForm()` / `EndModal()`
5. Running the Win32 message loop

Controls with anchoring flags are automatically repositioned and resized when the form window is resized at runtime. The loader also applies:
- **DPI scaling** — positions and sizes are scaled to the current monitor DPI
- **Per-control fonts** — custom font overrides with form font inheritance
- **Tooltips** — per-control tooltip text displayed on hover
- **ComboBox/ListBox items** — pre-populated with selected index
- **ProgressBar/TrackBar/UpDown** — range (min/max) and initial value applied via SendMessage
- **Picture images** — BMP/ICO files loaded and displayed
- **Tab stop and group** — WS_TABSTOP and WS_GROUP applied for keyboard navigation
- **Visibility and enabled state** — WS_VISIBLE and WS_DISABLED applied per control

### Example: Adding Event Handlers

```cpp
// Named control IDs (generated via File > Export Control IDs).
namespace Controls {
    inline constexpr int NameText = 101;
    inline constexpr int SubmitButton = 301;
    inline constexpr int CancelButton = 302;
}

auto events = FormDesigner::EventMap{};

events.onClick(Controls::SubmitButton, [](const FormDesigner::ClickEvent& e) {
    auto window = FormDesigner::FormWindow{e.formHwnd};
    auto name = window.GetTextBox(Controls::NameText).GetText();
    Win32::MessageBoxW(e.formHwnd, name.c_str(), L"Submitted", Win32::Mb_Ok);
});

events.onClick(Controls::CancelButton, [](const FormDesigner::ClickEvent& e) {
    Win32::DestroyWindow(e.formHwnd);
});

auto hwnd = FormDesigner::LoadForm(form, hInstance, events);
```

### Example: Modal Confirmation Dialog

The runner demonstrates showing a modal dialog that blocks the parent until dismissed:

```cpp
events.onClick(Controls::SubmitButton, [&](const FormDesigner::ClickEvent& e) {
    auto window = FormDesigner::FormWindow{e.formHwnd};
    auto name = window.GetTextBox(Controls::NameText).GetText();

    // Build and show a confirmation dialog.
    auto dlgEvents = FormDesigner::EventMap{};
    dlgEvents.onClick(1, [](auto& de) {
        FormDesigner::EndModal(de.formHwnd, FormDesigner::DialogResult::Ok);
    });
    dlgEvents.onClick(2, [](auto& de) {
        FormDesigner::EndModal(de.formHwnd, FormDesigner::DialogResult::Cancel);
    });

    auto result = FormDesigner::ShowModalForm(dialogForm, hInstance, dlgEvents, e.formHwnd);
    if (result == FormDesigner::DialogResult::Ok) { /* proceed */ }
});
```

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
