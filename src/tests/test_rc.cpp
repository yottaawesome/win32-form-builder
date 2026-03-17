#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static auto contains(const std::string& src, const std::string& sub) -> bool
{
	return src.find(sub) != std::string::npos;
}

// === DLU Conversion ===

TEST_CASE("PixelToDluX converts using base unit", "[rc]")
{
	DluMetrics m{ 7, 14 };
	// MulDiv(100, 4, 7) = 57
	REQUIRE(PixelToDluX(100, m) == 57);
	REQUIRE(PixelToDluX(0, m) == 0);
	REQUIRE(PixelToDluX(7, m) == 4);
}

TEST_CASE("PixelToDluY converts using base unit", "[rc]")
{
	DluMetrics m{ 7, 14 };
	// MulDiv(100, 8, 14) = 57
	REQUIRE(PixelToDluY(100, m) == 57);
	REQUIRE(PixelToDluY(0, m) == 0);
	REQUIRE(PixelToDluY(14, m) == 8);
}

TEST_CASE("EstimateDluMetrics returns defaults for empty font", "[rc]")
{
	auto m = EstimateDluMetrics(FontInfo{});
	REQUIRE(m.baseX == 7);
	REQUIRE(m.baseY == 14);
}

TEST_CASE("EstimateDluMetrics scales for custom font size", "[rc]")
{
	FontInfo font{ .family = L"Segoe UI", .size = 10 };
	auto m = EstimateDluMetrics(font);
	// baseX ≈ 10 * 0.875 = 9 (rounded), baseY ≈ 10 * 1.75 = 18 (rounded)
	REQUIRE(m.baseX == 9);
	REQUIRE(m.baseY == 18);
}

// === Resource Header ===

TEST_CASE("GenerateRcHeader produces IDD_DIALOG define", "[rc]")
{
	Form form;
	auto header = GenerateRcHeader(form);

	REQUIRE(contains(header, "#define IDD_DIALOG  101"));
	REQUIRE(contains(header, "#ifndef RESOURCE_H"));
	REQUIRE(contains(header, "#endif"));
}

TEST_CASE("GenerateRcHeader emits IDC defines for controls", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::Button, .id = 201 });
	form.controls.push_back(Control{
		.type = ControlType::TextBox, .id = 202 });

	auto header = GenerateRcHeader(form);

	REQUIRE(contains(header, "#define IDC_BUTTON_201  201"));
	REQUIRE(contains(header, "#define IDC_TEXTBOX_202  202"));
}

TEST_CASE("GenerateRcHeader skips controls without IDs", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::Label, .text = L"Static", .id = 0 });

	auto header = GenerateRcHeader(form);

	REQUIRE_FALSE(contains(header, "IDC_LABEL"));
}

// === RC Dialog Structure ===

TEST_CASE("GenerateRcDialog produces DIALOGEX header", "[rc]")
{
	Form form;
	form.title = L"Test Dialog";
	form.width = 640;
	form.height = 480;

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "IDD_DIALOG DIALOGEX 0, 0,"));
	REQUIRE(contains(rc, "CAPTION \"Test Dialog\""));
	REQUIRE(contains(rc, "BEGIN"));
	REQUIRE(contains(rc, "END"));
}

TEST_CASE("GenerateRcDialog uses default font when no custom font", "[rc]")
{
	Form form;
	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "FONT 8, \"MS Shell Dlg 2\", 400, 0, 0x1"));
}

TEST_CASE("GenerateRcDialog uses custom font with weight and italic", "[rc]")
{
	Form form;
	form.font = { .family = L"Segoe UI", .size = 10, .bold = true, .italic = true };

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "FONT 10, \"Segoe UI\", 700, 1, 0x1"));
}

TEST_CASE("GenerateRcDialog includes resource.h", "[rc]")
{
	Form form;
	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "#include \"resource.h\""));
	REQUIRE(contains(rc, "#include <windows.h>"));
}

// === Control Mapping ===

TEST_CASE("GenerateRcDialog emits LTEXT for Label", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::Label, .text = L"Hello", .rect = {10, 20, 100, 14}, .id = 1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "LTEXT"));
	REQUIRE(contains(rc, "\"Hello\""));
	REQUIRE(contains(rc, "IDC_LABEL_1"));
}

TEST_CASE("GenerateRcDialog emits CTEXT for centered Label", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::Label, .text = L"Center", .rect = {10, 20, 100, 14},
		.id = 1, .textAlign = TextAlign::Center });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "CTEXT"));
}

TEST_CASE("GenerateRcDialog emits RTEXT for right-aligned Label", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::Label, .text = L"Right", .rect = {10, 20, 100, 14},
		.id = 1, .textAlign = TextAlign::Right });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "RTEXT"));
}

TEST_CASE("GenerateRcDialog emits PUSHBUTTON for Button", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::Button, .text = L"OK", .rect = {10, 20, 50, 14}, .id = 1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "PUSHBUTTON"));
	REQUIRE(contains(rc, "\"OK\""));
}

TEST_CASE("GenerateRcDialog emits DEFPUSHBUTTON for Button with BS_DEFPUSHBUTTON", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::Button, .text = L"OK", .rect = {10, 20, 50, 14},
		.id = 1, .style = 0x1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "DEFPUSHBUTTON"));
}

TEST_CASE("GenerateRcDialog emits AUTOCHECKBOX for CheckBox", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::CheckBox, .text = L"Check", .rect = {10, 20, 80, 10}, .id = 1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "AUTOCHECKBOX"));
	REQUIRE(contains(rc, "\"Check\""));
}

TEST_CASE("GenerateRcDialog emits AUTORADIOBUTTON for RadioButton", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::RadioButton, .text = L"Option", .rect = {10, 20, 80, 10}, .id = 1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "AUTORADIOBUTTON"));
}

TEST_CASE("GenerateRcDialog emits GROUPBOX for GroupBox", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::GroupBox, .text = L"Group", .rect = {10, 20, 200, 100}, .id = 1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "GROUPBOX"));
	REQUIRE(contains(rc, "\"Group\""));
}

TEST_CASE("GenerateRcDialog emits EDITTEXT for TextBox", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::TextBox, .rect = {10, 20, 100, 14}, .id = 1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "EDITTEXT"));
	REQUIRE(contains(rc, "ES_AUTOHSCROLL"));
}

TEST_CASE("GenerateRcDialog emits COMBOBOX with dropdown height", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::ComboBox, .rect = {10, 20, 100, 21}, .id = 1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "COMBOBOX"));
	REQUIRE(contains(rc, "CBS_DROPDOWNLIST"));
}

TEST_CASE("GenerateRcDialog emits LISTBOX for ListBox", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::ListBox, .rect = {10, 20, 100, 80}, .id = 1 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "LISTBOX"));
	REQUIRE(contains(rc, "LBS_STANDARD"));
}

TEST_CASE("GenerateRcDialog emits generic CONTROL for common controls", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::ProgressBar, .rect = {10, 20, 200, 14}, .id = 1 });
	form.controls.push_back(Control{
		.type = ControlType::ListView, .rect = {10, 40, 200, 100}, .id = 2 });
	form.controls.push_back(Control{
		.type = ControlType::TreeView, .rect = {10, 150, 200, 100}, .id = 3 });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "\"msctls_progress32\""));
	REQUIRE(contains(rc, "\"SysListView32\""));
	REQUIRE(contains(rc, "\"SysTreeView32\""));
	REQUIRE(contains(rc, "CONTROL"));
}

TEST_CASE("GenerateRcDialog emits EXSTYLE when form has exStyle", "[rc]")
{
	Form form;
	form.exStyle = 0x100; // WS_EX_CONTEXTHELP

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "EXSTYLE 0x00000100"));
}

TEST_CASE("GenerateRcDialog omits EXSTYLE when zero", "[rc]")
{
	Form form;
	form.exStyle = 0;

	auto rc = GenerateRcDialog(form);

	REQUIRE_FALSE(contains(rc, "EXSTYLE"));
}

TEST_CASE("GenerateRcDialog uses IDC_STATIC for controls without IDs", "[rc]")
{
	Form form;
	form.controls.push_back(Control{
		.type = ControlType::Label, .text = L"Info", .rect = {10, 20, 100, 14} });

	auto rc = GenerateRcDialog(form);

	REQUIRE(contains(rc, "IDC_STATIC"));
}
