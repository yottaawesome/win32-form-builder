#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static auto contains(const std::string& haystack, const std::string& needle) -> bool
{
	return haystack.find(needle) != std::string::npos;
}

// === Schema defaults ===

TEST_CASE("Control bindField defaults to empty", "[databinding]")
{
	auto ctrl = Control{};
	REQUIRE(ctrl.bindField.empty());
}

TEST_CASE("Form bindStruct defaults to empty", "[databinding]")
{
	auto form = Form{};
	REQUIRE(form.bindStruct.empty());
}

// === Parser round-trip ===

TEST_CASE("Parse and serialize bindField on control", "[databinding]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": [
			{ "type": "TextBox", "text": "Name", "rect": [10, 10, 200, 25], "id": 101, "bindField": "firstName" }
		]
	})");

	auto form = ParseForm(json);
	REQUIRE(form.controls.size() == 1);
	REQUIRE(form.controls[0].bindField == "firstName");

	auto output = SerializeForm(form);
	REQUIRE(contains(output, "\"bindField\""));
	REQUIRE(contains(output, "firstName"));

	auto reparsed = ParseForm(output);
	REQUIRE(reparsed.controls[0].bindField == "firstName");
}

TEST_CASE("Parse and serialize bindStruct on form", "[databinding]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"bindStruct": "UserInfo",
		"controls": []
	})");

	auto form = ParseForm(json);
	REQUIRE(form.bindStruct == "UserInfo");

	auto output = SerializeForm(form);
	REQUIRE(contains(output, "\"bindStruct\""));
	REQUIRE(contains(output, "UserInfo"));

	auto reparsed = ParseForm(output);
	REQUIRE(reparsed.bindStruct == "UserInfo");
}

TEST_CASE("Empty bindField is not serialized", "[databinding]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": [
			{ "type": "Button", "text": "OK", "rect": [10, 10, 80, 30], "id": 101 }
		]
	})");

	auto form = ParseForm(json);
	auto output = SerializeForm(form);
	REQUIRE_FALSE(contains(output, "bindField"));
}

TEST_CASE("Empty bindStruct is not serialized", "[databinding]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": []
	})");

	auto form = ParseForm(json);
	auto output = SerializeForm(form);
	REQUIRE_FALSE(contains(output, "bindStruct"));
}

// === Codegen: PopulateForm and ReadForm ===

TEST_CASE("Codegen emits PopulateForm and ReadForm when bindings are present", "[databinding]")
{
	auto form = Form{};
	form.title = L"BindTest";
	form.bindStruct = "UserInfo";

	auto tb = Control{};
	tb.type = ControlType::TextBox;
	tb.text = L"Name";
	tb.rect = { 10, 10, 200, 25 };
	tb.id = 101;
	tb.bindField = "name";
	form.controls.push_back(tb);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "void PopulateForm(HWND hwnd, const UserInfo& data)"));
	REQUIRE(contains(code, "void ReadForm(HWND hwnd, UserInfo& data)"));
}

TEST_CASE("Codegen omits binding functions when no bindStruct", "[databinding]")
{
	auto form = Form{};
	form.title = L"NoStruct";

	auto tb = Control{};
	tb.type = ControlType::TextBox;
	tb.text = L"Name";
	tb.rect = { 10, 10, 200, 25 };
	tb.id = 101;
	tb.bindField = "name";
	form.controls.push_back(tb);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "PopulateForm"));
	REQUIRE_FALSE(contains(code, "ReadForm"));
}

TEST_CASE("Codegen omits binding functions when no controls have bindField", "[databinding]")
{
	auto form = Form{};
	form.title = L"NoFields";
	form.bindStruct = "UserInfo";

	auto tb = Control{};
	tb.type = ControlType::TextBox;
	tb.text = L"Name";
	tb.rect = { 10, 10, 200, 25 };
	tb.id = 101;
	form.controls.push_back(tb);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "PopulateForm"));
	REQUIRE_FALSE(contains(code, "ReadForm"));
}

TEST_CASE("Codegen emits SetDlgItemTextW for TextBox binding", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "MyData";

	auto tb = Control{};
	tb.type = ControlType::TextBox;
	tb.text = L"Field";
	tb.rect = { 10, 10, 200, 25 };
	tb.id = 101;
	tb.bindField = "username";
	form.controls.push_back(tb);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "SetDlgItemTextW(hwnd, IDC_TEXTBOX_101, data.username.c_str())"));
	REQUIRE(contains(code, "GetDlgItemTextW(hwnd, IDC_TEXTBOX_101, buf, 1024)"));
	REQUIRE(contains(code, "data.username = buf"));
}

TEST_CASE("Codegen emits BM_SETCHECK for CheckBox binding", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "Settings";

	auto cb = Control{};
	cb.type = ControlType::CheckBox;
	cb.text = L"Enable";
	cb.rect = { 10, 10, 100, 20 };
	cb.id = 201;
	cb.bindField = "enabled";
	form.controls.push_back(cb);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "BM_SETCHECK"));
	REQUIRE(contains(code, "data.enabled"));
	REQUIRE(contains(code, "BM_GETCHECK"));
}

TEST_CASE("Codegen emits SetDlgItemInt for TrackBar binding", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "Prefs";

	auto tb = Control{};
	tb.type = ControlType::TrackBar;
	tb.text = L"Volume";
	tb.rect = { 10, 10, 200, 30 };
	tb.id = 301;
	tb.bindField = "volume";
	form.controls.push_back(tb);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "SetDlgItemInt(hwnd, IDC_TRACKBAR_301"));
	REQUIRE(contains(code, "data.volume"));
	REQUIRE(contains(code, "GetDlgItemInt(hwnd, IDC_TRACKBAR_301"));
}

TEST_CASE("Codegen emits CB_SETCURSEL for ComboBox binding", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "Config";

	auto cb = Control{};
	cb.type = ControlType::ComboBox;
	cb.text = L"Choice";
	cb.rect = { 10, 10, 150, 25 };
	cb.id = 401;
	cb.bindField = "selectedOption";
	form.controls.push_back(cb);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "CB_SETCURSEL"));
	REQUIRE(contains(code, "data.selectedOption"));
	REQUIRE(contains(code, "CB_GETCURSEL"));
}

TEST_CASE("Codegen emits LB_SETCURSEL for ListBox binding", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "Config";

	auto lb = Control{};
	lb.type = ControlType::ListBox;
	lb.text = L"Items";
	lb.rect = { 10, 10, 150, 100 };
	lb.id = 501;
	lb.bindField = "selectedItem";
	form.controls.push_back(lb);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "LB_SETCURSEL"));
	REQUIRE(contains(code, "data.selectedItem"));
	REQUIRE(contains(code, "LB_GETCURSEL"));
}

TEST_CASE("Codegen emits struct skeleton comment", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "UserData";

	auto tb = Control{};
	tb.type = ControlType::TextBox;
	tb.rect = { 10, 10, 200, 25 };
	tb.id = 101;
	tb.bindField = "name";
	form.controls.push_back(tb);

	auto cb = Control{};
	cb.type = ControlType::CheckBox;
	cb.rect = { 10, 40, 100, 20 };
	cb.id = 102;
	cb.bindField = "active";
	form.controls.push_back(cb);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "// struct UserData"));
	REQUIRE(contains(code, "//     std::wstring name;"));
	REQUIRE(contains(code, "//     bool active;"));
}

TEST_CASE("Codegen forward declares binding functions", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "TestStruct";

	auto tb = Control{};
	tb.type = ControlType::TextBox;
	tb.rect = { 10, 10, 200, 25 };
	tb.id = 101;
	tb.bindField = "field1";
	form.controls.push_back(tb);

	auto code = GenerateCode(form, false);

	// Forward declarations should appear before function body.
	auto fwdPos = code.find("void PopulateForm(HWND hwnd, const TestStruct& data);");
	auto bodyPos = code.find("void PopulateForm(HWND hwnd, const TestStruct& data)\n{");
	REQUIRE(fwdPos != std::string::npos);
	REQUIRE(bodyPos != std::string::npos);
	REQUIRE(fwdPos < bodyPos);
}

TEST_CASE("Codegen skips controls with unsupported bind types", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "Data";

	// GroupBox has BindType::None — should not appear in binding functions.
	auto gb = Control{};
	gb.type = ControlType::GroupBox;
	gb.text = L"Group";
	gb.rect = { 10, 10, 200, 100 };
	gb.id = 601;
	gb.bindField = "group";
	form.controls.push_back(gb);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "PopulateForm"));
	REQUIRE_FALSE(contains(code, "ReadForm"));
}

TEST_CASE("Codegen handles mixed bound and unbound controls", "[databinding]")
{
	auto form = Form{};
	form.bindStruct = "MixedData";

	auto tb1 = Control{};
	tb1.type = ControlType::TextBox;
	tb1.rect = { 10, 10, 200, 25 };
	tb1.id = 101;
	tb1.bindField = "name";
	form.controls.push_back(tb1);

	// Unbound button — should not appear in binding code.
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 40, 80, 30 };
	btn.id = 102;
	form.controls.push_back(btn);

	auto tb2 = Control{};
	tb2.type = ControlType::CheckBox;
	tb2.rect = { 10, 80, 100, 20 };
	tb2.id = 103;
	tb2.bindField = "active";
	form.controls.push_back(tb2);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "data.name"));
	REQUIRE(contains(code, "data.active"));

	// Extract just the PopulateForm body — unbound button should not be in it.
	auto popStart = code.find("void PopulateForm(");
	REQUIRE(popStart != std::string::npos);
	auto popEnd = code.find("void ReadForm(");
	REQUIRE(popEnd != std::string::npos);
	auto populateBody = code.substr(popStart, popEnd - popStart);
	REQUIRE_FALSE(contains(populateBody, "IDC_BUTTON_102"));
}
