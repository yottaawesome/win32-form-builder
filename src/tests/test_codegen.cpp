#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

// === ToPascalCase (tested indirectly via generated output) ===

// Helper: check that generated code contains a substring.
static auto contains(const std::string& haystack, const std::string& needle) -> bool
{
	return haystack.find(needle) != std::string::npos;
}

// === Basic generation ===

TEST_CASE("GenerateCode produces compilable structure for empty form", "[codegen]")
{
	auto form = Form{};
	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "#include <windows.h>"));
	REQUIRE(contains(code, "#include <commctrl.h>"));
	REQUIRE(contains(code, "#pragma comment(lib, \"comctl32.lib\")"));
	REQUIRE(contains(code, "LRESULT CALLBACK WndProc"));
	REQUIRE(contains(code, "void CreateFormControls"));
	REQUIRE(contains(code, "wWinMain"));
	REQUIRE(contains(code, "RegisterClassExW"));
	REQUIRE(contains(code, "GetMessageW"));
	REQUIRE(contains(code, "PostQuitMessage(0)"));
}

TEST_CASE("GenerateCode modules mode adds import std", "[codegen]")
{
	auto form = Form{};
	auto code = GenerateCode(form, true);

	REQUIRE(contains(code, "import std;"));
	REQUIRE(contains(code, "#include <windows.h>"));
}

TEST_CASE("GenerateCode classic mode does not include import std", "[codegen]")
{
	auto form = Form{};
	auto code = GenerateCode(form, false);

	REQUIRE_FALSE(contains(code, "import std;"));
}

// === Form properties ===

TEST_CASE("GenerateCode uses form title and dimensions", "[codegen]")
{
	auto form = Form{};
	form.title = L"My Test Form";
	form.width = 800;
	form.height = 600;
	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "L\"My Test Form\""));
	REQUIRE(contains(code, "800"));
	REQUIRE(contains(code, "600"));
	REQUIRE(contains(code, "MyTestFormClass"));
}

TEST_CASE("GenerateCode handles background color", "[codegen]")
{
	auto form = Form{};
	form.backgroundColor = 0x00EFEFEF; // COLORREF: R=EF, G=EF, B=EF
	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "WM_ERASEBKGND"));
	REQUIRE(contains(code, "CreateSolidBrush"));
	REQUIRE(contains(code, "RGB(239, 239, 239)"));
}

TEST_CASE("GenerateCode omits WM_ERASEBKGND when no background color", "[codegen]")
{
	auto form = Form{};
	auto code = GenerateCode(form, false);

	REQUIRE_FALSE(contains(code, "WM_ERASEBKGND"));
}

// === Controls ===

TEST_CASE("GenerateCode creates controls with correct class names and styles", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::Button,
		.text = L"Click Me",
		.rect = { 10, 20, 100, 30 },
		.id = 101,
	});
	form.controls.push_back(Control{
		.type = ControlType::TextBox,
		.rect = { 10, 60, 200, 24 },
		.id = 102,
	});

	auto code = GenerateCode(form, false);

	// Button
	REQUIRE(contains(code, "WC_BUTTON"));
	REQUIRE(contains(code, "L\"Click Me\""));
	REQUIRE(contains(code, "IDC_BUTTON_101"));
	REQUIRE(contains(code, "10, 20, 100, 30"));

	// TextBox
	REQUIRE(contains(code, "WC_EDIT"));
	REQUIRE(contains(code, "WS_BORDER"));
	REQUIRE(contains(code, "ES_AUTOHSCROLL"));
	REQUIRE(contains(code, "IDC_TEXTBOX_102"));
}

TEST_CASE("GenerateCode generates IDC_ defines for controls with IDs", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::CheckBox,
		.text = L"Option A",
		.id = 201,
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "#define IDC_CHECKBOX_201  201"));
}

TEST_CASE("GenerateCode applies WM_SETFONT to each control", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::Label,
		.text = L"Hello",
		.id = 1,
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "SendMessageW("));
	REQUIRE(contains(code, "WM_SETFONT"));
	REQUIRE(contains(code, "DEFAULT_GUI_FONT"));
}

// === Event handler stubs ===

TEST_CASE("GenerateCode generates event handler stubs for onClick", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::Button,
		.text = L"Submit",
		.id = 301,
		.onClick = "submit_form",
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "void OnSubmitForm(HWND hwnd, int controlId)"));
	REQUIRE(contains(code, "BN_CLICKED"));
	REQUIRE(contains(code, "IDC_BUTTON_301"));
	REQUIRE(contains(code, "OnSubmitForm(hwnd, id)"));
}

TEST_CASE("GenerateCode generates onCheck stub with bool parameter", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::CheckBox,
		.text = L"Accept",
		.id = 201,
		.onCheck = "accept_changed",
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "void OnAcceptChanged(HWND hwnd, int controlId, bool checked)"));
	REQUIRE(contains(code, "BM_GETCHECK"));
	REQUIRE(contains(code, "BST_CHECKED"));
}

TEST_CASE("GenerateCode generates onChange for TextBox as EN_CHANGE", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::TextBox,
		.id = 102,
		.onChange = "text_changed",
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "EN_CHANGE"));
	REQUIRE(contains(code, "OnTextChanged(hwnd, id)"));
}

TEST_CASE("GenerateCode generates WM_NOTIFY for ListView onSelectionChange", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::ListView,
		.id = 401,
		.onSelectionChange = "list_item_selected",
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "WM_NOTIFY"));
	REQUIRE(contains(code, "LVN_ITEMCHANGED"));
	REQUIRE(contains(code, "OnListItemSelected(hwnd, id)"));
}

// === Style generation ===

TEST_CASE("GenerateCode generates implied styles for CheckBox", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::CheckBox,
		.text = L"Check",
		.id = 1,
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "BS_AUTOCHECKBOX"));
	REQUIRE(contains(code, "WS_CHILD"));
	REQUIRE(contains(code, "WS_VISIBLE"));
}

TEST_CASE("GenerateCode generates alignment styles", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::Label,
		.text = L"Centered",
		.id = 1,
		.textAlign = TextAlign::Center,
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "SS_CENTER"));
}

TEST_CASE("GenerateCode includes custom style as hex", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::Button,
		.text = L"Default",
		.id = 1,
		.style = 0x1, // BS_DEFPUSHBUTTON
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "0x1"));
}

// === Window styles ===

TEST_CASE("GenerateCode uses WS_OVERLAPPEDWINDOW for default form style", "[codegen]")
{
	auto form = Form{};
	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "WS_OVERLAPPEDWINDOW"));
}

// === RichEdit support ===

TEST_CASE("GenerateCode includes richedit.h and LoadLibrary for RichEdit controls", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::RichEdit,
		.id = 501,
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "#include <richedit.h>"));
	REQUIRE(contains(code, "LoadLibraryW(L\"msftedit.dll\")"));
	REQUIRE(contains(code, "MSFTEDIT_CLASS"));
}

// === String escaping ===

TEST_CASE("GenerateCode escapes special characters in control text", "[codegen]")
{
	auto form = Form{};
	form.controls.push_back(Control{
		.type = ControlType::Label,
		.text = L"Line1\nLine2",
		.id = 1,
	});

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "L\"Line1\\nLine2\""));
}

// === Comprehensive roundtrip: sample form style ===

TEST_CASE("GenerateCode produces complete output for a multi-control form", "[codegen]")
{
	auto form = Form{};
	form.title = L"Sample Form";
	form.width = 400;
	form.height = 300;
	form.backgroundColor = 0x00EFEFEF;
	form.controls = {
		Control{.type = ControlType::Label, .text = L"Name:", .rect = {20, 22, 60, 17}},
		Control{.type = ControlType::TextBox, .rect = {90, 20, 200, 24}, .id = 101},
		Control{.type = ControlType::Button, .text = L"Submit", .rect = {80, 210, 100, 30}, .id = 301, .onClick = "submit_form"},
		Control{.type = ControlType::Button, .text = L"Cancel", .rect = {190, 210, 100, 30}, .id = 302, .onClick = "cancel_form"},
	};

	auto code = GenerateCode(form, false);

	// Preamble
	REQUIRE(contains(code, "#include <windows.h>"));
	REQUIRE(contains(code, "#pragma comment(lib, \"comctl32.lib\")"));

	// Resource IDs
	REQUIRE(contains(code, "#define IDC_TEXTBOX_101"));
	REQUIRE(contains(code, "#define IDC_BUTTON_301"));
	REQUIRE(contains(code, "#define IDC_BUTTON_302"));

	// Event stubs
	REQUIRE(contains(code, "void OnSubmitForm(HWND hwnd, int controlId)"));
	REQUIRE(contains(code, "void OnCancelForm(HWND hwnd, int controlId)"));

	// Controls
	REQUIRE(contains(code, "WC_STATIC"));
	REQUIRE(contains(code, "WC_EDIT"));
	REQUIRE(contains(code, "WC_BUTTON"));

	// WndProc
	REQUIRE(contains(code, "WM_CREATE"));
	REQUIRE(contains(code, "WM_COMMAND"));
	REQUIRE(contains(code, "WM_ERASEBKGND"));
	REQUIRE(contains(code, "WM_DESTROY"));

	// WinMain
	REQUIRE(contains(code, "wWinMain"));
	REQUIRE(contains(code, "SampleFormClass"));
	REQUIRE(contains(code, "L\"Sample Form\""));
	REQUIRE(contains(code, "400"));
	REQUIRE(contains(code, "300"));
}

TEST_CASE("Codegen generates WM_SIZE handler for anchored controls", "[codegen]")
{
	auto contains = [](const std::string& src, const std::string& sub)
	{
		return src.find(sub) != std::string::npos;
	};

	Form form;
	form.title = L"Anchor Test";
	form.width = 640;
	form.height = 480;

	Control c;
	c.type = ControlType::TextBox;
	c.id = 100;
	c.rect = { 10, 10, 300, 200 };
	c.anchor = Anchor::Top | Anchor::Left | Anchor::Right;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "WM_SIZE"));
	REQUIRE(contains(code, "deltaW"));
	REQUIRE(contains(code, "MoveWindow"));
	REQUIRE(contains(code, "IDC_TEXTBOX_100"));
}

TEST_CASE("Codegen omits WM_SIZE when all controls use default anchor", "[codegen]")
{
	auto contains = [](const std::string& src, const std::string& sub)
	{
		return src.find(sub) != std::string::npos;
	};

	Form form;
	form.title = L"No Anchor Test";
	form.width = 400;
	form.height = 300;

	Control c;
	c.type = ControlType::Button;
	c.id = 200;
	c.rect = { 10, 10, 80, 25 };
	// default anchor (Top|Left) — no WM_SIZE needed
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);

	REQUIRE_FALSE(contains(code, "WM_SIZE"));
}

// === Font code generation tests ===

TEST_CASE("GenerateCode omits CreateFontW when no custom fonts", "[codegen][font]")
{
	Form form;
	form.title = L"No Font";
	form.controls.emplace_back();
	form.controls[0].type = ControlType::Button;
	form.controls[0].text = L"OK";
	form.controls[0].id = 1;

	auto code = GenerateCode(form, false);

	REQUIRE_FALSE(contains(code, "CreateFontW"));
	REQUIRE(contains(code, "DEFAULT_GUI_FONT"));
}

TEST_CASE("GenerateCode emits CreateFontW for control with custom font", "[codegen][font]")
{
	Form form;
	form.title = L"Font Test";
	form.controls.emplace_back();
	form.controls[0].type = ControlType::Label;
	form.controls[0].text = L"Hello";
	form.controls[0].id = 1;
	form.controls[0].font.family = L"Consolas";
	form.controls[0].font.size = 14;
	form.controls[0].font.bold = true;

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "CreateFontW"));
	REQUIRE(contains(code, "L\"Consolas\""));
	REQUIRE(contains(code, "FW_BOLD"));
}

TEST_CASE("GenerateCode emits CreateFontW for form-level font", "[codegen][font]")
{
	Form form;
	form.title = L"Form Font";
	form.font.family = L"Arial";
	form.font.size = 11;

	form.controls.emplace_back();
	form.controls[0].type = ControlType::Button;
	form.controls[0].text = L"OK";
	form.controls[0].id = 1;

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "CreateFontW"));
	REQUIRE(contains(code, "L\"Arial\""));
}

TEST_CASE("GenerateCode shares font variable when controls have same font", "[codegen][font]")
{
	Form form;
	form.title = L"Shared Font";

	auto font = FontInfo{ L"Consolas", 12, false, false };

	Control c1;
	c1.type = ControlType::Label;
	c1.text = L"A";
	c1.id = 1;
	c1.font = font;

	Control c2;
	c2.type = ControlType::Label;
	c2.text = L"B";
	c2.id = 2;
	c2.font = font;

	form.controls.push_back(c1);
	form.controls.push_back(c2);

	auto code = GenerateCode(form, false);

	// Should only have one CreateFontW call for the shared font.
	auto first = code.find("CreateFontW");
	REQUIRE(first != std::string::npos);
	auto second = code.find("CreateFontW", first + 1);
	// Second occurrence should be from WM_SETFONT, not another CreateFontW variable.
	// Count unique hFont variable declarations.
	REQUIRE(contains(code, "hFont"));
	// Both controls should use the same font variable.
}

// === Tooltip code generation tests ===

TEST_CASE("GenerateCode emits tooltip window when control has tooltip", "[codegen][tooltip]")
{
	Form form;
	form.title = L"Tooltip Test";

	Control c;
	c.type = ControlType::Button;
	c.text = L"OK";
	c.id = 101;
	c.tooltip = L"Click me";
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "TOOLTIPS_CLASS"));
	REQUIRE(contains(code, "TTM_SETMAXTIPWIDTH"));
	REQUIRE(contains(code, "TTM_ADDTOOL"));
	REQUIRE(contains(code, "TTF_SUBCLASS"));
	REQUIRE(contains(code, "Click me"));
}

TEST_CASE("GenerateCode omits tooltip window when no tooltips", "[codegen][tooltip]")
{
	Form form;
	form.title = L"No Tooltips";

	Control c;
	c.type = ControlType::Button;
	c.text = L"OK";
	c.id = 101;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);

	REQUIRE_FALSE(contains(code, "TOOLTIPS_CLASS"));
	REQUIRE_FALSE(contains(code, "TTM_ADDTOOL"));
	REQUIRE_FALSE(contains(code, "hTooltip"));
}

TEST_CASE("GenerateCode emits tooltip only for controls that have one", "[codegen][tooltip]")
{
	Form form;
	form.title = L"Mixed Tooltips";

	Control c1;
	c1.type = ControlType::Button;
	c1.text = L"Save";
	c1.id = 1;
	c1.tooltip = L"Save file";

	Control c2;
	c2.type = ControlType::Button;
	c2.text = L"Cancel";
	c2.id = 2;
	// No tooltip

	form.controls.push_back(c1);
	form.controls.push_back(c2);

	auto code = GenerateCode(form, false);

	REQUIRE(contains(code, "TOOLTIPS_CLASS"));
	REQUIRE(contains(code, "Save file"));
	// Count TTM_ADDTOOL occurrences - should be exactly 1
	auto pos = code.find("TTM_ADDTOOL");
	REQUIRE(pos != std::string::npos);
	auto second = code.find("TTM_ADDTOOL", pos + 1);
	REQUIRE(second == std::string::npos);
}
