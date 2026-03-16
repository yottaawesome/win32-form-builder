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
