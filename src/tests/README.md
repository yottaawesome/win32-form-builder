# Tests

Unit tests for the [formbuilder](../formbuilder/README.md) library using [Catch2](https://github.com/catchorg/Catch2) v3.

## Test Suites

| File | Tests | What It Covers |
|------|-------|----------------|
| `test_schema.cpp` | 17 | `ControlType` enum, `ClassNameFor`, `ImpliedStyleFor`, `AlignmentStyleFor`, default values (anchor, groupId) |
| `test_parser_serializer.cpp` | 32 | JSON → `Form` parsing, `Form` → JSON serialization, round-trip fidelity, edge cases, anchor serialization |
| `test_events.cpp` | 5 | `EventMap` registration and dispatch for all 7 event types |
| `test_helpers.cpp` | 39 | Designer helpers — hit testing, resize handles, snap guides, control type display names, validation, tab order, grouping, recent files |
| `test_codegen.cpp` | 29 | C++ code generation — preamble, styles, controls, events, IDC defines, RichEdit, escaping, anchoring |
| **Total** | **122** | **437 assertions** |

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
