# Win32 Form Designer

A visual form designer for Win32 applications. Design forms by dragging and dropping controls onto a canvas, configure properties, and export the result as JSON, standalone C++ source code, or `.rc` dialog resources.

## Features

### Design Surface
- **23 control types**: Button, CheckBox, RadioButton, Label, TextBox, GroupBox, ListBox, ComboBox, ProgressBar, TrackBar, DateTimePicker, TabControl, ListView, TreeView, UpDown, RichEdit, MonthCalendar, Link, IPAddress, HotKey, Picture, Separator, Animation
- **Drag and drop** controls from the toolbox onto the canvas
- **Resize handles** on selected controls (8-point)
- **Multi-select** with Ctrl+Click
- **Snap-to-align guides** — controls snap to edges and centers of other controls during drag
- **Grid snap** with configurable grid size and visible grid overlay
- **Ruler guides** — drag from graduated rulers to create alignment guide lines
- **Form boundary** — visual border showing the form's actual dimensions
- **Keyboard nudge** — arrow keys move selected controls by 1px (or grid size with Snap)
- **Per-Monitor V2 DPI awareness** — scales correctly on high-DPI displays

### Property Editor
- Edit control properties: type, text, ID, position, size, tab index, text alignment, anchor, locked state, visible, enabled
- **Font properties**: per-control font family, size, bold, italic (inherits form font by default)
- **Per-control tooltips**: tooltip text displayed at runtime on hover
- **Value, Min, Max**: initial value and range for ProgressBar, TrackBar, UpDown controls
- **ComboBox/ListBox items**: edit items list and selected index
- **Picture image path**: browse for BMP/ICO files
- **Data binding**: bind struct (form-level) and bind field (control-level) for PopulateForm/ReadForm codegen
- **Validation metadata**: required, min/max length, regex pattern, min/max range (per control type)
- **Accessibility**: accessible name, description, tab stop, group start properties
- Edit form properties: title, width, height, background color (with color picker), window styles, font, bind struct
- 7 event types: onClick, onChange, onDoubleClick, onSelectionChange, onFocus, onBlur, onCheck
- **Property validation** — invalid values are rejected with pink/red visual error highlighting and status bar messages
- Scrollable panel with dynamic layout (conditional fields reflow based on control type)

### Alignment & Layout
- **Format menu** with 11 alignment operations:
  - 6 align: Left, Right, Top, Bottom, Center Horizontal, Center Vertical
  - 2 distribute: Horizontal, Vertical spacing
  - 3 match size: Width, Height, Both
- Alignment toolbar and right-click context menu access

### Editing
- **Undo/Redo** (Ctrl+Z / Ctrl+Y) — full snapshot-based history
- **Cut/Copy/Paste/Duplicate** (Ctrl+X/C/V/D)
- **Delete** selected controls (Del key)
- **Select All** (Ctrl+A)
- **Control grouping** (Ctrl+G / Ctrl+Shift+G) — group related controls so they select and move as a unit, shown with dashed border
- **Control locking** — prevent accidental edits to finalized controls
- **Control anchoring** — configure resize behavior per control (Top, Bottom, Left, Right edge flags)
- **Tab order editor** — visual mode (View menu) with click-to-assign tab indices, shown as numbered badges
- **Z-Order panel** — reorder controls with multi-select, inline rename, delete, and keyboard shortcuts
- **Right-click context menu** — Cut, Copy, Paste, Duplicate, Delete, Lock/Unlock, Group/Ungroup, Bring to Front, Send to Back

### File Operations
- **New / Open / Save / Save As** with JSON format
- **Recent Files** — quick access to recently opened forms (persisted across sessions)
- **Built-in templates** — Login, Settings, Data Entry, About, Search starter layouts (File → New from Template)
- **Export to C++** — generates a standalone `.cpp` file with `WinMain`, `WndProc`, `CreateWindowExW` calls, event handler stubs, and data binding helpers (choice of classic `#include` or C++20 `import std;` style)
- **Export to .rc** — generates Win32 dialog resource files with pixel-to-DLU conversion and `resource.h` header
- **Preview** (F5) — live in-process preview of the form

### Tools
- **Check Accessibility** (Tools menu) — runs 7-rule audit: missing labels, button/label access keys, picture without accessible name, duplicate tab indices, tab stop disabled, radio not grouped

### View Options
- Show/hide grid overlay and snap-to-grid (View menu)
- Show/hide rulers with draggable guide lines
- Clear all guides
- **Dark mode** — toggle between light and dark themes (persisted to `designer.ini`)
- **Tab order editor** — visual mode for assigning tab indices by clicking controls
- **Settings persistence** — grid, snap, rulers, theme, and window placement saved to `designer.ini`

### UI
- **Toolbar** with common actions and tooltips (New, Open, Save, Undo, Redo, Cut, Copy, Paste, Delete, Preview)
- **Status bar** showing selection info, cursor position, and file state

## Building

### Prerequisites
- **Visual Studio 2022** (v17.x) or later with the **Desktop development with C++** workload
- **Platform Toolset**: v145
- **C++ Language Standard**: C++20 (`/std:c++latest`)
- **vcpkg** (for nlohmann-json dependency, resolved via the formbuilder project)

### Build from Visual Studio
1. Open `src\win32-form-builder.sln`
2. Set the configuration to **Debug | x64**
3. Set **win32-form-designer** as the startup project
4. Build the solution (Ctrl+Shift+B)

### Build from command line
```
MSBuild src\win32-form-builder.sln /p:Configuration=Debug /p:Platform=x64 /m
```

The output binary is at `src\x64\Debug\win32-form-designer.exe`.

## Running

```
win32-form-designer.exe [path-to-form.json]
```

- With no arguments: opens with a blank form
- With a JSON file path: opens that form for editing

### Debugging in Visual Studio
1. Right-click **win32-form-designer** → Properties → Debugging
2. Set **Command Arguments** to the path of a JSON form file (e.g. `$(SolutionDir)win32-form-runner\sample-form.json`)
3. Press F5 to start debugging

## Architecture

The designer is built as a C++20 module (`designer`) with partitions that build on the `formbuilder` library:

| Partition | File | Purpose |
|-----------|------|---------|
| `:state` | `state.ixx` | Data structures, constants, menu IDs, `DesignState` struct |
| `:hit_testing` | `hit_testing.ixx` | Control hit testing and resize handle detection |
| `:rendering` | `rendering.ixx` | Selection rendering, snap guides, rulers, canvas painting |
| `:settings` | `settings.ixx` | Theme persistence, window placement, and user settings |
| `:helpers` | `helpers.ixx` | Validation, tab order, grouping, recent files, utility functions |
| `:properties` | `properties.ixx` | Property panel creation, updates, change handlers, and dynamic reflow |
| `:canvas` | `canvas.ixx` | Canvas window procedure, control lifecycle, drag/resize, clipboard |
| `:fileops` | `fileops.ixx` | File dialogs, save/open/new/export operations |
| `:alignment` | `alignment.ixx` | Multi-control alignment, distribution, and size matching |
| `:templates` | `templates.ixx` | Built-in starter form templates |

The main module interface (`designer.ixx`) creates the design surface, menu bar (File, Edit, Format, Templates, Tools, View), accelerator table, toolbar, and status bar.

Win32 API wrappers are centralised in the `formbuilder:win32` partition — the only file that includes `<Windows.h>`. All designer partitions access Win32 APIs via `import formbuilder`.

### Partition dependency chain
```
state → hit_testing → rendering → settings → helpers → properties → canvas → fileops → alignment → templates → designer.ixx
```

### Key design patterns
- **Single `<Windows.h>` include** — only the formbuilder's `win32.ixx` includes Windows headers; all other files use `import formbuilder`
- **Win32:: namespace** — all Win32 types, functions, and constants are wrapped in the `Win32` namespace with descriptive names (e.g. `Win32::Styles::Child`, `Win32::Messages::Paint`)
- **DesignState** — single struct holding all mutable state, stored via `GWLP_USERDATA` on each window
- **Snapshot undo** — entire `Form` is copied on each undoable action
- **Dynamic property reflow** — conditional property rows are repositioned at runtime to eliminate gaps when hidden

## JSON Format

Forms are saved as JSON files. See `sample-form.json` in the runner project for an example. Key fields:

```json
{
  "title": "My Form",
  "width": 640,
  "height": 480,
  "backgroundColor": "#EFEFEF",
  "style": 13565952,
  "bindStruct": "FormData",
  "controls": [
    {
      "type": "Button",
      "text": "&Click Me",
      "id": 101,
      "rect": [10, 20, 100, 30],
      "onClick": "handle_click",
      "textAlign": "center",
      "tooltip": "Perform action",
      "tabStop": true
    }
  ],
  "guides": [
    { "horizontal": true, "position": 100 }
  ]
}
```

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+N | New form |
| Ctrl+O | Open form |
| Ctrl+S | Save |
| Ctrl+Shift+S | Save As |
| F5 | Preview form |
| Ctrl+Z | Undo |
| Ctrl+Y | Redo |
| Ctrl+X | Cut |
| Ctrl+C | Copy |
| Ctrl+V | Paste |
| Ctrl+D | Duplicate |
| Ctrl+A | Select All |
| Ctrl+G | Group selected controls |
| Ctrl+Shift+G | Ungroup selected controls |
| Del | Delete selected |
| Arrow keys | Nudge selected controls |
| Esc | Cancel placement / exit tab order mode |
