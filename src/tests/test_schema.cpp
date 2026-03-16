#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

TEST_CASE("ClassNameFor returns correct Win32 class names", "[schema]")
{
    REQUIRE(std::wstring(ClassNameFor(ControlType::Button))  == L"Button");
    REQUIRE(std::wstring(ClassNameFor(ControlType::CheckBox)) == L"Button");
    REQUIRE(std::wstring(ClassNameFor(ControlType::RadioButton)) == L"Button");
    REQUIRE(std::wstring(ClassNameFor(ControlType::Label))   == L"Static");
    REQUIRE(std::wstring(ClassNameFor(ControlType::TextBox)) == L"Edit");
    REQUIRE(std::wstring(ClassNameFor(ControlType::GroupBox)) == L"Button");
    REQUIRE(std::wstring(ClassNameFor(ControlType::ListBox)) == L"ListBox");
    REQUIRE(std::wstring(ClassNameFor(ControlType::ComboBox)) == L"ComboBox");
}

TEST_CASE("ClassNameFor returns nullptr for Window type", "[schema]")
{
    REQUIRE(ClassNameFor(ControlType::Window) == nullptr);
}

TEST_CASE("ClassNameFor handles common control types", "[schema]")
{
    REQUIRE(ClassNameFor(ControlType::ProgressBar) != nullptr);
    REQUIRE(ClassNameFor(ControlType::TrackBar) != nullptr);
    REQUIRE(ClassNameFor(ControlType::DateTimePicker) != nullptr);
    REQUIRE(ClassNameFor(ControlType::TabControl) != nullptr);
    REQUIRE(ClassNameFor(ControlType::ListView) != nullptr);
    REQUIRE(ClassNameFor(ControlType::TreeView) != nullptr);
    REQUIRE(ClassNameFor(ControlType::UpDown) != nullptr);
    REQUIRE(ClassNameFor(ControlType::RichEdit) != nullptr);
}

TEST_CASE("ImpliedStyleFor returns non-zero for types with implied styles", "[schema]")
{
    REQUIRE(ImpliedStyleFor(ControlType::CheckBox) != 0);
    REQUIRE(ImpliedStyleFor(ControlType::RadioButton) != 0);
    REQUIRE(ImpliedStyleFor(ControlType::GroupBox) != 0);
    REQUIRE(ImpliedStyleFor(ControlType::TextBox) != 0);
    REQUIRE(ImpliedStyleFor(ControlType::ListBox) != 0);
    REQUIRE(ImpliedStyleFor(ControlType::ComboBox) != 0);
    REQUIRE(ImpliedStyleFor(ControlType::ListView) != 0);
    REQUIRE(ImpliedStyleFor(ControlType::TreeView) != 0);
    REQUIRE(ImpliedStyleFor(ControlType::RichEdit) != 0);
}

TEST_CASE("ImpliedStyleFor returns 0 for types with no implied style", "[schema]")
{
    REQUIRE(ImpliedStyleFor(ControlType::Button) == 0);
    REQUIRE(ImpliedStyleFor(ControlType::Window) == 0);
    REQUIRE(ImpliedStyleFor(ControlType::Label) == 0);  // SS_LEFT is 0
    REQUIRE(ImpliedStyleFor(ControlType::ProgressBar) == 0);
    REQUIRE(ImpliedStyleFor(ControlType::TrackBar) == 0);
    REQUIRE(ImpliedStyleFor(ControlType::UpDown) == 0);
}

TEST_CASE("Rect has sensible default values", "[schema]")
{
    Rect r;
    REQUIRE(r.x == 0);
    REQUIRE(r.y == 0);
    REQUIRE(r.width == 100);
    REQUIRE(r.height == 25);
}

TEST_CASE("Control has sensible default values", "[schema]")
{
    Control c;
    REQUIRE(c.type == ControlType::Window);
    REQUIRE(c.text.empty());
    REQUIRE(c.id == 0);
    REQUIRE(c.style == 0);
    REQUIRE(c.exStyle == 0);
    REQUIRE(c.tabIndex == 0);
    REQUIRE(c.onClick.empty());
    REQUIRE(c.onChange.empty());
    REQUIRE(c.onDoubleClick.empty());
    REQUIRE(c.onSelectionChange.empty());
    REQUIRE(c.onFocus.empty());
    REQUIRE(c.onBlur.empty());
    REQUIRE(c.groupId == 0);
    REQUIRE(c.onCheck.empty());
    REQUIRE(c.textAlign == TextAlign::Left);
    REQUIRE(c.anchor == (Anchor::Top | Anchor::Left));
    REQUIRE(c.children.empty());
}

TEST_CASE("Form has sensible default values", "[schema]")
{
    Form f;
    REQUIRE(f.title == L"Untitled");
    REQUIRE(f.width == 640);
    REQUIRE(f.height == 480);
    REQUIRE(f.backgroundColor == -1);
    REQUIRE(f.controls.empty());
}

TEST_CASE("AlignmentStyleFor returns correct style bits for Labels", "[schema]")
{
    REQUIRE(AlignmentStyleFor(ControlType::Label, TextAlign::Left) == 0); // SS_LEFT is 0
    REQUIRE(AlignmentStyleFor(ControlType::Label, TextAlign::Center) != 0);
    REQUIRE(AlignmentStyleFor(ControlType::Label, TextAlign::Right) != 0);
    REQUIRE(AlignmentStyleFor(ControlType::Label, TextAlign::Center) !=
            AlignmentStyleFor(ControlType::Label, TextAlign::Right));
}

TEST_CASE("AlignmentStyleFor returns correct style bits for TextBox", "[schema]")
{
    REQUIRE(AlignmentStyleFor(ControlType::TextBox, TextAlign::Left) == 0);
    REQUIRE(AlignmentStyleFor(ControlType::TextBox, TextAlign::Center) != 0);
    REQUIRE(AlignmentStyleFor(ControlType::TextBox, TextAlign::Right) != 0);
}

TEST_CASE("AlignmentStyleFor returns correct style bits for Button types", "[schema]")
{
    REQUIRE(AlignmentStyleFor(ControlType::Button, TextAlign::Center) != 0);
    REQUIRE(AlignmentStyleFor(ControlType::CheckBox, TextAlign::Right) != 0);
    REQUIRE(AlignmentStyleFor(ControlType::RadioButton, TextAlign::Left) != 0);
}

TEST_CASE("AlignmentStyleFor returns 0 for unsupported control types", "[schema]")
{
    REQUIRE(AlignmentStyleFor(ControlType::GroupBox, TextAlign::Center) == 0);
    REQUIRE(AlignmentStyleFor(ControlType::ListBox, TextAlign::Right) == 0);
    REQUIRE(AlignmentStyleFor(ControlType::ProgressBar, TextAlign::Center) == 0);
}

TEST_CASE("Control locked defaults to false", "[schema]")
{
    auto c = Control{};
    REQUIRE(c.locked == false);
}

TEST_CASE("DesignerGuide defaults", "[schema]")
{
    auto g = DesignerGuide{};
    REQUIRE(g.horizontal == false);
    REQUIRE(g.position == 0);
}

TEST_CASE("Form guides defaults to empty", "[schema]")
{
    auto f = Form{};
    REQUIRE(f.guides.empty());
}

// === FontInfo tests ===

TEST_CASE("FontInfo default is not set", "[schema][font]")
{
    FontInfo fi;
    REQUIRE_FALSE(fi.isSet());
    REQUIRE(fi.family.empty());
    REQUIRE(fi.size == 0);
    REQUIRE(fi.bold == false);
    REQUIRE(fi.italic == false);
}

TEST_CASE("FontInfo with family is set", "[schema][font]")
{
    FontInfo fi;
    fi.family = L"Arial";
    REQUIRE(fi.isSet());
}

TEST_CASE("FontInfo with size is set", "[schema][font]")
{
    FontInfo fi;
    fi.size = 12;
    REQUIRE(fi.isSet());
}

TEST_CASE("FontInfo with bold only is not set (bold needs family or size)", "[schema][font]")
{
    FontInfo fi;
    fi.bold = true;
    REQUIRE_FALSE(fi.isSet());
}

TEST_CASE("Control has default font", "[schema][font]")
{
    Control c;
    REQUIRE_FALSE(c.font.isSet());
}

TEST_CASE("Form has default font", "[schema][font]")
{
    Form f;
    REQUIRE_FALSE(f.font.isSet());
}

TEST_CASE("ResolveFont returns system default when nothing set", "[schema][font]")
{
    FontInfo ctrl, form;
    auto resolved = ResolveFont(ctrl, form);
    REQUIRE(resolved.family == DefaultFontFamily);
    REQUIRE(resolved.size == DefaultFontSize);
    REQUIRE(resolved.bold == false);
    REQUIRE(resolved.italic == false);
}

TEST_CASE("ResolveFont uses form font when control has no override", "[schema][font]")
{
    FontInfo ctrl;
    FontInfo form;
    form.family = L"Consolas";
    form.size = 14;
    form.bold = true;

    auto resolved = ResolveFont(ctrl, form);
    REQUIRE(resolved.family == L"Consolas");
    REQUIRE(resolved.size == 14);
    REQUIRE(resolved.bold == true);
    REQUIRE(resolved.italic == false);
}

TEST_CASE("ResolveFont control overrides form", "[schema][font]")
{
    FontInfo ctrl;
    ctrl.family = L"Arial";
    ctrl.size = 10;

    FontInfo form;
    form.family = L"Consolas";
    form.size = 14;
    form.bold = true;

    auto resolved = ResolveFont(ctrl, form);
    REQUIRE(resolved.family == L"Arial");
    REQUIRE(resolved.size == 10);
    // When control font is set, its bold/italic override the form's.
    REQUIRE(resolved.bold == false);
    REQUIRE(resolved.italic == false);
}

TEST_CASE("ResolveFont partial control override with bold/italic", "[schema][font]")
{
    FontInfo ctrl;
    ctrl.size = 20;
    ctrl.bold = true;  // Explicitly setting bold on control.

    FontInfo form;
    form.family = L"Times New Roman";
    form.italic = true;

    auto resolved = ResolveFont(ctrl, form);
    REQUIRE(resolved.family == L"Times New Roman");  // From form (control has no family).
    REQUIRE(resolved.size == 20);                     // From control.
    REQUIRE(resolved.bold == true);                   // From control.
    REQUIRE(resolved.italic == false);                // Control overrides, italic defaults to false.
}
