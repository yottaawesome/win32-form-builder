#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;
import designer;

using namespace FormDesigner;
using namespace Designer;

// Helper: create a DesignState with controls for testing.
static DesignState MakeTestState(std::vector<Control> controls)
{
    DesignState state;
    state.form.controls = std::move(controls);

    for (auto& c : state.form.controls)
    {
        ControlEntry entry;
        entry.hwnd = nullptr;
        entry.control = &c;
        state.entries.push_back(entry);
    }
    return state;
}

// === IsSelected / SingleSelection ===

TEST_CASE("IsSelected returns true for selected index", "[helpers]")
{
    auto state = MakeTestState({});
    state.selection.insert(2);
    REQUIRE(IsSelected(state, 2));
}

TEST_CASE("IsSelected returns false for unselected index", "[helpers]")
{
    auto state = MakeTestState({});
    state.selection.insert(1);
    REQUIRE_FALSE(IsSelected(state, 3));
}

TEST_CASE("SingleSelection returns index when exactly one selected", "[helpers]")
{
    auto state = MakeTestState({});
    state.selection.insert(5);
    REQUIRE(SingleSelection(state) == 5);
}

TEST_CASE("SingleSelection returns -1 when no selection", "[helpers]")
{
    auto state = MakeTestState({});
    REQUIRE(SingleSelection(state) == -1);
}

TEST_CASE("SingleSelection returns -1 when multiple selected", "[helpers]")
{
    auto state = MakeTestState({});
    state.selection.insert(1);
    state.selection.insert(3);
    REQUIRE(SingleSelection(state) == -1);
}

// === HitTest ===

TEST_CASE("HitTest finds control at coordinates", "[helpers]")
{
    Control c;
    c.rect = { 10, 20, 100, 50 };
    auto state = MakeTestState({ c });

    REQUIRE(HitTest(state, 50, 40) == 0);
}

TEST_CASE("HitTest returns -1 for empty area", "[helpers]")
{
    Control c;
    c.rect = { 10, 20, 100, 50 };
    auto state = MakeTestState({ c });

    REQUIRE(HitTest(state, 200, 200) == -1);
}

TEST_CASE("HitTest returns topmost (last) control at overlapping position", "[helpers]")
{
    Control c1, c2;
    c1.rect = { 0, 0, 100, 100 };
    c2.rect = { 50, 50, 100, 100 };
    auto state = MakeTestState({ c1, c2 });

    // Point at (60, 60) is in both controls; control 1 (index 1) should win.
    REQUIRE(HitTest(state, 60, 60) == 1);
}

TEST_CASE("HitTest returns -1 for empty state", "[helpers]")
{
    auto state = MakeTestState({});
    REQUIRE(HitTest(state, 50, 50) == -1);
}

// === GetHandleAnchors ===

TEST_CASE("GetHandleAnchors computes 8 handle positions", "[helpers]")
{
    Rect r = { 100, 200, 80, 40 };
    Win32::POINT anchors[8];
    GetHandleAnchors(r, anchors);

    // Top-left handle should be at (100 - HANDLE_HALF, 200 - HANDLE_HALF).
    REQUIRE(anchors[0].x == 100 - 3);
    REQUIRE(anchors[0].y == 200 - 3);

    // Bottom-right handle should be at (180 - HANDLE_HALF, 240 - HANDLE_HALF).
    REQUIRE(anchors[7].x == 180 - 3);
    REQUIRE(anchors[7].y == 240 - 3);

    // Top-center handle x should be at midpoint.
    REQUIRE(anchors[1].x == 140 - 3);
}

// === CursorForHandle ===

TEST_CASE("CursorForHandle returns correct cursor for diagonal handles", "[helpers]")
{
    // Handles 0 and 7 are NW-SE diagonal.
    REQUIRE(CursorForHandle(0) == Win32::Cursors::SizeNWSE);
    REQUIRE(CursorForHandle(7) == Win32::Cursors::SizeNWSE);

    // Handles 2 and 5 are NE-SW diagonal.
    REQUIRE(CursorForHandle(2) == Win32::Cursors::SizeNESW);
    REQUIRE(CursorForHandle(5) == Win32::Cursors::SizeNESW);
}

TEST_CASE("CursorForHandle returns correct cursor for edge handles", "[helpers]")
{
    // Handles 1 and 6 are vertical.
    REQUIRE(CursorForHandle(1) == Win32::Cursors::SizeNS);
    REQUIRE(CursorForHandle(6) == Win32::Cursors::SizeNS);

    // Handles 3 and 4 are horizontal.
    REQUIRE(CursorForHandle(3) == Win32::Cursors::SizeWE);
    REQUIRE(CursorForHandle(4) == Win32::Cursors::SizeWE);
}

// === ApplyResize ===

TEST_CASE("ApplyResize from bottom-right increases size", "[helpers]")
{
    Rect r = { 10, 20, 100, 50 };
    Win32::POINT startPos = { 10, 20 };
    Win32::SIZE startSize = { 100, 50 };

    ApplyResize(r, 7, 30, 20, startPos, startSize);  // Handle 7 = bottom-right

    REQUIRE(r.x == 10);       // Position unchanged.
    REQUIRE(r.y == 20);
    REQUIRE(r.width == 130);   // Grew by 30.
    REQUIRE(r.height == 70);   // Grew by 20.
}

TEST_CASE("ApplyResize from top-left moves position and changes size", "[helpers]")
{
    Rect r = { 50, 50, 100, 80 };
    Win32::POINT startPos = { 50, 50 };
    Win32::SIZE startSize = { 100, 80 };

    ApplyResize(r, 0, -10, -10, startPos, startSize);  // Handle 0 = top-left

    REQUIRE(r.x == 40);       // Moved left 10.
    REQUIRE(r.y == 40);       // Moved up 10.
    REQUIRE(r.width == 110);   // Grew by 10.
    REQUIRE(r.height == 90);   // Grew by 10.
}

TEST_CASE("ApplyResize enforces minimum size", "[helpers]")
{
    Rect r = { 50, 50, 30, 30 };
    Win32::POINT startPos = { 50, 50 };
    Win32::SIZE startSize = { 30, 30 };

    // Drag bottom-right way to the left to shrink below minimum.
    ApplyResize(r, 7, -200, -200, startPos, startSize);

    REQUIRE(r.width >= 10);
    REQUIRE(r.height >= 10);
}

// === ControlTypeDisplayName ===

TEST_CASE("ControlTypeDisplayName returns display names", "[helpers]")
{
    REQUIRE(std::wstring(ControlTypeDisplayName(ControlType::Button))     == L"Button");
    REQUIRE(std::wstring(ControlTypeDisplayName(ControlType::CheckBox))   == L"CheckBox");
    REQUIRE(std::wstring(ControlTypeDisplayName(ControlType::TextBox))    == L"TextBox");
    REQUIRE(std::wstring(ControlTypeDisplayName(ControlType::ListView))   == L"ListView");
    REQUIRE(std::wstring(ControlTypeDisplayName(ControlType::RichEdit))   == L"RichEdit");
    REQUIRE(std::wstring(ControlTypeDisplayName(ControlType::Window))     == L"Window");
}

// === ColorRefToHex / HexToColorRef ===

TEST_CASE("ColorRefToHex converts COLORREF to hex string", "[helpers]")
{
    // COLORREF 0x00FF8040 = R=0x40, G=0x80, B=0xFF
    // Wait, COLORREF is 0x00BBGGRR.
    // So 0x004080FF → R=0xFF, G=0x80, B=0x40 → "#FF8040"
    REQUIRE(ColorRefToHex(0x004080FF) == L"#FF8040");
}

TEST_CASE("ColorRefToHex returns empty for -1", "[helpers]")
{
    REQUIRE(ColorRefToHex(-1).empty());
}

TEST_CASE("HexToColorRef converts hex string to COLORREF", "[helpers]")
{
    REQUIRE(HexToColorRef(L"#FF8040") == static_cast<int>(0x004080FF));
}

TEST_CASE("HexToColorRef returns -1 for invalid input", "[helpers]")
{
    REQUIRE(HexToColorRef(L"invalid") == -1);
    REQUIRE(HexToColorRef(L"#12345") == -1);
    REQUIRE(HexToColorRef(L"") == -1);
}

TEST_CASE("ColorRefToHex and HexToColorRef roundtrip", "[helpers]")
{
    int original = 0x00336699;  // R=0x99, G=0x66, B=0x33
    auto hex = ColorRefToHex(original);
    auto result = HexToColorRef(hex);
    REQUIRE(result == original);
}

// === NextControlId ===

TEST_CASE("NextControlId returns 1 for empty controls", "[helpers]")
{
    auto state = MakeTestState({});
    REQUIRE(NextControlId(state) == 1);
}

TEST_CASE("NextControlId returns max+1", "[helpers]")
{
    Control c1, c2, c3;
    c1.id = 5;
    c2.id = 3;
    c3.id = 10;
    auto state = MakeTestState({ c1, c2, c3 });
    REQUIRE(NextControlId(state) == 11);
}

// === PushUndo ===

TEST_CASE("PushUndo pushes form snapshot onto undo stack", "[helpers]")
{
    auto state = MakeTestState({});
    state.form.title = L"Before";

    PushUndo(state);

    REQUIRE(state.undoStack.size() == 1);
    REQUIRE(state.undoStack[0].title == L"Before");
}

TEST_CASE("PushUndo clears redo stack", "[helpers]")
{
    auto state = MakeTestState({});
    state.redoStack.push_back(Form{});
    state.redoStack.push_back(Form{});

    PushUndo(state);

    REQUIRE(state.redoStack.empty());
}

// === SnapValue / SnapRectToGrid ===

TEST_CASE("SnapValue rounds to nearest grid point", "[helpers]")
{
    REQUIRE(SnapValue(0, 10) == 0);
    REQUIRE(SnapValue(3, 10) == 0);
    REQUIRE(SnapValue(5, 10) == 10);
    REQUIRE(SnapValue(7, 10) == 10);
    REQUIRE(SnapValue(14, 10) == 10);
    REQUIRE(SnapValue(15, 10) == 20);
    REQUIRE(SnapValue(100, 10) == 100);
}

TEST_CASE("SnapValue works with different grid sizes", "[helpers]")
{
    REQUIRE(SnapValue(7, 5) == 5);
    REQUIRE(SnapValue(13, 5) == 15);
    REQUIRE(SnapValue(8, 8) == 8);
    REQUIRE(SnapValue(12, 8) == 16);
}

TEST_CASE("SnapRectToGrid snaps x and y", "[helpers]")
{
    Rect r = { 13, 27, 100, 50 };
    SnapRectToGrid(r, 10);
    REQUIRE(r.x == 10);
    REQUIRE(r.y == 30);
    REQUIRE(r.width == 100);   // Width unchanged.
    REQUIRE(r.height == 50);   // Height unchanged.
}

// === FindAlignGuides ===

TEST_CASE("FindAlignGuides snaps when within threshold", "[helpers]")
{
    Control existing;
    existing.rect = { 100, 200, 80, 30 };
    auto state = MakeTestState({ existing });

    // Moving control near left edge = 103 (within 4px of existing left = 100).
    Rect moving = { 103, 50, 60, 25 };
    FindAlignGuides(state, moving);

    // Should snap x to 100.
    REQUIRE(moving.x == 100);
    REQUIRE(!state.guides.empty());
}

TEST_CASE("FindAlignGuides does not snap when far away", "[helpers]")
{
    Control existing;
    existing.rect = { 100, 200, 80, 30 };
    auto state = MakeTestState({ existing });

    Rect moving = { 300, 50, 60, 25 };
    FindAlignGuides(state, moving);

    REQUIRE(moving.x == 300);
    REQUIRE(state.guides.empty());
}

TEST_CASE("FindAlignGuides skips selected controls", "[helpers]")
{
    Control c1, c2;
    c1.rect = { 100, 200, 80, 30 };
    c2.rect = { 300, 200, 80, 30 };
    auto state = MakeTestState({ c1, c2 });

    // Select control 0 so it's skipped.
    state.selection.insert(0);

    // Moving rect near c1's position but c1 is selected.
    Rect moving = { 102, 50, 60, 25 };
    FindAlignGuides(state, moving);

    // Should NOT snap to c1 since it's selected.
    REQUIRE(moving.x == 102);
}
