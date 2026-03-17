# Tests

Unit tests for the [formbuilder](../formbuilder/README.md) library using [Catch2](https://github.com/catchorg/Catch2) v3.

## Test Suites

| File | Tests | What It Covers |
|------|-------|----------------|
| `test_schema.cpp` | 25 | `ControlType` enum, `ClassNameFor`, `ImpliedStyleFor`, `AlignmentStyleFor`, default values (anchor, groupId) |
| `test_parser_serializer.cpp` | 56 | JSON → `Form` parsing, `Form` → JSON serialization, round-trip fidelity, edge cases, anchor serialization |
| `test_events.cpp` | 5 | `EventMap` registration and dispatch for all 7 event types |
| `test_errors.cpp` | 20 | `FormException` construction, error codes, `TryLoadFormFromFile` / `TryLoadForm` / `TryShowModalForm` returning `std::expected` |
| `test_helpers.cpp` | 48 | Designer helpers — hit testing, resize handles, snap guides, control type display names, validation, tab order, grouping, recent files |
| `test_codegen.cpp` | 33 | C++ code generation — preamble, styles, controls, events, IDC defines, RichEdit, escaping, anchoring, data binding |
| `test_alignment.cpp` | 15 | Multi-control alignment (6 align, 2 distribute, 3 match size), minimum selection requirements |
| `test_dpi.cpp` | 7 | DPI scaling calculations, control rect scaling, font size scaling, manifest configuration |
| `test_rc.cpp` | 26 | `.rc` dialog export — keyword vs generic controls, styles, pixel-to-DLU, header generation, escaping |
| `test_visible.cpp` | 14 | Per-control visible property — schema defaults, round-trip, codegen WS_VISIBLE, RC style flags, hatching overlay |
| `test_validation.cpp` | 18 | Validation metadata — required, min/max length, pattern, min/max range, round-trip, codegen comments |
| `test_picture.cpp` | 14 | Picture control — image path, BMP/ICO detection, round-trip, codegen (LoadImageW, STM_SETIMAGE) |
| `test_databinding.cpp` | 18 | Data binding — bindStruct/bindField, round-trip, PopulateForm/ReadForm codegen, type inference |
| `test_disabled.cpp` | 12 | Enabled/disabled property — schema defaults, round-trip, codegen WS_DISABLED, RC style flags |
| `test_accessibility.cpp` | 32 | Accessibility — tabStop, groupStart, accessibleName/Description, round-trip, codegen, RC styles, 7 audit rules |
| `test_value.cpp` | 18 | Value property — ProgressBar/TrackBar/UpDown, round-trip, codegen PBM/TBM/UDM messages, range emit |
| `test_form_visible_enabled.cpp` | 18 | Form-level visible/enabled — schema defaults, round-trip, codegen WS_DISABLED/ShowWindow, RC/loader |
| `test_controls.cpp` | 35 | Typed control wrappers — compile-time type traits, FormWindow access, live TextBox/CheckBox/RadioButton/ComboBox/ListBox/ProgressBar/TrackBar/UpDown operations |
| `test_control_ids.cpp` | 14 | Generated control IDs — `GenerateControlIds()` output format, naming, uniqueness, empty form, duplicate handling |
| `test_modal.cpp` | 7 | Modal dialog support — `ShowModalForm`, `EndModal`, `DialogResult` values, parent disable/enable |
| `test_hotreload.cpp` | 9 | Hot reload — `EnableHotReload`, `DisableHotReload`, file change detection, timer lifecycle |
| `test_wrapper_events.cpp` | 11 | Wrapper-based event binding — `OnClick`, `OnChange`, `OnCheck` on typed wrappers, `RequireEvents` guard |
| `test_messagebox.cpp` | 12 | Message box helpers — `ShowInfo`, `ShowError`, `ShowWarning`, `AskYesNo`, `AskYesNoCancel`, `AskOkCancel` constants |
| **Total** | **467** | **1183 assertions across 467 test cases** |

## Running

### From command line
```
src\x64\Debug\tests.exe
```

### With a specific reporter
```
src\x64\Debug\tests.exe --reporter compact
```

### Run a single test
```
src\x64\Debug\tests.exe "ParseForm produces correct controls"
```

### Run tests by tag
```
src\x64\Debug\tests.exe [value]
src\x64\Debug\tests.exe [accessibility]
```

### List all tests
```
src\x64\Debug\tests.exe --list-tests
```

## Building

### Prerequisites
- **Visual Studio 2022** (v17.x) or later
- **vcpkg** with `catch2` — resolved automatically via `vcpkg.json` manifest
- The **formbuilder** library (project reference)

### Build from command line
```
MSBuild src\tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```

> **Note:** After adding new test files, you may need to clean the test project first for the build to pick them up:
> ```
> MSBuild src\tests\tests.vcxproj /t:Clean /p:Configuration=Debug /p:Platform=x64
> MSBuild src\tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
> ```

## Architecture

- `main.cpp` — Catch2 session entry point (no custom main logic)
- Each `test_*.cpp` file imports the `formbuilder` module and tests one partition area
- Tests that exercise designer helpers also compile `designer.*.ixx` files directly (the designer module is not a library, so test files include the module sources)

## Adding a New Test File

1. Create `test_yourfeature.cpp` in `src/tests/`
2. Add it to `tests.vcxproj` under `<ItemGroup>` with `<ClCompile>`
3. Use `#include <catch2/catch_test_macros.hpp>` and `import formbuilder;`
4. Clean and rebuild the test project
