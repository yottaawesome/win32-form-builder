#include <catch2/catch_test_macros.hpp>

import formbuilder;
import std;

using namespace FormDesigner;

// Helper: check if a string contains a substring.
static bool contains(const std::string& haystack, const std::string& needle)
{
	return haystack.find(needle) != std::string::npos;
}

// ===== Schema Defaults =====

TEST_CASE("tabStop defaults to true", "[accessibility][schema]")
{
	auto ctrl = Control{};
	REQUIRE(ctrl.tabStop == true);
}

TEST_CASE("groupStart defaults to false", "[accessibility][schema]")
{
	auto ctrl = Control{};
	REQUIRE(ctrl.groupStart == false);
}

TEST_CASE("accessibleName defaults to empty", "[accessibility][schema]")
{
	auto ctrl = Control{};
	REQUIRE(ctrl.accessibleName.empty());
}

TEST_CASE("accessibleDescription defaults to empty", "[accessibility][schema]")
{
	auto ctrl = Control{};
	REQUIRE(ctrl.accessibleDescription.empty());
}

TEST_CASE("IsInteractiveControl returns true for interactive types", "[accessibility][schema]")
{
	REQUIRE(IsInteractiveControl(ControlType::Button));
	REQUIRE(IsInteractiveControl(ControlType::CheckBox));
	REQUIRE(IsInteractiveControl(ControlType::RadioButton));
	REQUIRE(IsInteractiveControl(ControlType::TextBox));
	REQUIRE(IsInteractiveControl(ControlType::ListBox));
	REQUIRE(IsInteractiveControl(ControlType::ComboBox));
	REQUIRE(IsInteractiveControl(ControlType::TreeView));
	REQUIRE(IsInteractiveControl(ControlType::ListView));
	REQUIRE(IsInteractiveControl(ControlType::RichEdit));
	REQUIRE(IsInteractiveControl(ControlType::HotKey));
	REQUIRE(IsInteractiveControl(ControlType::IPAddress));
}

TEST_CASE("IsInteractiveControl returns false for non-interactive types", "[accessibility][schema]")
{
	REQUIRE_FALSE(IsInteractiveControl(ControlType::Label));
	REQUIRE_FALSE(IsInteractiveControl(ControlType::GroupBox));
	REQUIRE_FALSE(IsInteractiveControl(ControlType::Picture));
	REQUIRE_FALSE(IsInteractiveControl(ControlType::Separator));
	REQUIRE_FALSE(IsInteractiveControl(ControlType::Animation));
}

// ===== Round-trip Parse/Serialize =====

TEST_CASE("tabStop=false round-trips through JSON", "[accessibility][parser][serializer]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button };
	ctrl.tabStop = false;
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls[0].tabStop == false);
}

TEST_CASE("tabStop=true is not serialized (default)", "[accessibility][serializer]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button };
	ctrl.tabStop = true;
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	REQUIRE_FALSE(contains(json, "tabStop"));
}

TEST_CASE("groupStart=true round-trips through JSON", "[accessibility][parser][serializer]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::RadioButton };
	ctrl.groupStart = true;
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls[0].groupStart == true);
}

TEST_CASE("groupStart=false is not serialized (default)", "[accessibility][serializer]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::RadioButton };
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	REQUIRE_FALSE(contains(json, "groupStart"));
}

TEST_CASE("accessibleName round-trips through JSON", "[accessibility][parser][serializer]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Picture };
	ctrl.accessibleName = L"Company Logo";
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	REQUIRE(contains(json, "accessibleName"));
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls[0].accessibleName == L"Company Logo");
}

TEST_CASE("accessibleDescription round-trips through JSON", "[accessibility][parser][serializer]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::TextBox };
	ctrl.accessibleDescription = L"Enter your email address";
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	REQUIRE(contains(json, "accessibleDescription"));
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls[0].accessibleDescription == L"Enter your email address");
}

// ===== Codegen =====

TEST_CASE("Codegen emits WS_TABSTOP for interactive controls", "[accessibility][codegen]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button, .text = L"OK" };
	ctrl.id = 1;
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "WS_TABSTOP"));
}

TEST_CASE("Codegen omits WS_TABSTOP for non-interactive controls", "[accessibility][codegen]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Label, .text = L"Name:" };
	ctrl.id = 1;
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "WS_TABSTOP"));
}

TEST_CASE("Codegen omits WS_TABSTOP when tabStop=false", "[accessibility][codegen]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button, .text = L"OK" };
	ctrl.id = 1;
	ctrl.tabStop = false;
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "WS_TABSTOP"));
}

TEST_CASE("Codegen emits WS_GROUP when groupStart=true", "[accessibility][codegen]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::RadioButton, .text = L"Option A" };
	ctrl.id = 1;
	ctrl.groupStart = true;
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "WS_GROUP"));
}

TEST_CASE("Codegen emits accessible name as comment", "[accessibility][codegen]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Picture };
	ctrl.id = 1;
	ctrl.accessibleName = L"Logo";
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "Accessible name: \"Logo\""));
}

TEST_CASE("Codegen emits accessible description as comment", "[accessibility][codegen]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::TextBox };
	ctrl.id = 1;
	ctrl.accessibleDescription = L"Email field";
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "Accessible description: \"Email field\""));
}

// ===== RC Export =====

TEST_CASE("RC emits WS_TABSTOP for interactive keyword controls", "[accessibility][rc]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::TextBox };
	ctrl.id = 1;
	form.controls.push_back(ctrl);

	auto rc = GenerateRcDialog(form);
	REQUIRE(contains(rc, "WS_TABSTOP"));
}

TEST_CASE("RC omits WS_TABSTOP when tabStop=false", "[accessibility][rc]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::TextBox };
	ctrl.id = 1;
	ctrl.tabStop = false;
	form.controls.push_back(ctrl);

	auto rc = GenerateRcDialog(form);
	REQUIRE_FALSE(contains(rc, "WS_TABSTOP"));
}

TEST_CASE("RC emits WS_GROUP when groupStart=true", "[accessibility][rc]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::RadioButton, .text = L"Yes" };
	ctrl.id = 1;
	ctrl.groupStart = true;
	form.controls.push_back(ctrl);

	auto rc = GenerateRcDialog(form);
	REQUIRE(contains(rc, "WS_GROUP"));
}

// ===== Accessibility Audit =====

TEST_CASE("Audit: no warnings for well-formed form", "[accessibility][audit]")
{
	auto form = Form{};
	form.controls.push_back(Control{ .type = ControlType::Label, .text = L"&Name:" });
	form.controls.push_back(Control{ .type = ControlType::TextBox, .id = 1 });
	auto warnings = CheckAccessibility(form);
	REQUIRE(warnings.empty());
}

TEST_CASE("Audit: warns about interactive control without preceding label", "[accessibility][audit]")
{
	auto form = Form{};
	form.controls.push_back(Control{ .type = ControlType::TextBox, .id = 1 });
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "No preceding label")) found = true;
	REQUIRE(found);
}

TEST_CASE("Audit: warns about button without access key", "[accessibility][audit]")
{
	auto form = Form{};
	form.controls.push_back(Control{ .type = ControlType::Button, .text = L"OK", .id = 1 });
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "no access key")) found = true;
	REQUIRE(found);
}

TEST_CASE("Audit: no warning for button with access key", "[accessibility][audit]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button, .text = L"&OK", .id = 1 };
	ctrl.tabStop = true;
	auto label = Control{ .type = ControlType::Label, .text = L"&X" };
	form.controls.push_back(label);
	form.controls.push_back(ctrl);
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "no access key") && w.controlId == 1) found = true;
	REQUIRE_FALSE(found);
}

TEST_CASE("Audit: warns about label without access key", "[accessibility][audit]")
{
	auto form = Form{};
	form.controls.push_back(Control{ .type = ControlType::Label, .text = L"Name:" });
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "no access key")) found = true;
	REQUIRE(found);
}

TEST_CASE("Audit: error for picture without accessible name", "[accessibility][audit]")
{
	auto form = Form{};
	form.controls.push_back(Control{ .type = ControlType::Picture, .id = 1 });
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "no accessible name") && w.level == AccessibilityLevel::Error) found = true;
	REQUIRE(found);
}

TEST_CASE("Audit: no error for picture with accessible name", "[accessibility][audit]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Picture, .id = 1 };
	ctrl.accessibleName = L"Logo";
	form.controls.push_back(ctrl);
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "no accessible name")) found = true;
	REQUIRE_FALSE(found);
}

TEST_CASE("Audit: warns about duplicate tabIndex values", "[accessibility][audit]")
{
	auto form = Form{};
	auto ctrl1 = Control{ .type = ControlType::Button, .text = L"&A", .id = 1 };
	ctrl1.tabIndex = 1;
	auto ctrl2 = Control{ .type = ControlType::Button, .text = L"&B", .id = 2 };
	ctrl2.tabIndex = 1;
	auto lbl = Control{ .type = ControlType::Label, .text = L"&X" };
	form.controls.push_back(lbl);
	form.controls.push_back(ctrl1);
	form.controls.push_back(lbl);
	form.controls.push_back(ctrl2);
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "Duplicate tabIndex")) found = true;
	REQUIRE(found);
}

TEST_CASE("Audit: warns about disabled tab stop on interactive control", "[accessibility][audit]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::TextBox, .id = 1 };
	ctrl.tabStop = false;
	ctrl.accessibleName = L"Field";
	form.controls.push_back(ctrl);
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "Tab stop disabled")) found = true;
	REQUIRE(found);
}

TEST_CASE("Audit: warns about ungrouped radio button", "[accessibility][audit]")
{
	auto form = Form{};
	auto radio = Control{ .type = ControlType::RadioButton, .text = L"&Option", .id = 1 };
	auto lbl = Control{ .type = ControlType::Label, .text = L"&X" };
	form.controls.push_back(lbl);
	form.controls.push_back(radio);
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "not in a group")) found = true;
	REQUIRE(found);
}

TEST_CASE("Audit: no warning for radio button with groupStart", "[accessibility][audit]")
{
	auto form = Form{};
	auto radio = Control{ .type = ControlType::RadioButton, .text = L"&Yes", .id = 1 };
	radio.groupStart = true;
	auto lbl = Control{ .type = ControlType::Label, .text = L"&X" };
	form.controls.push_back(lbl);
	form.controls.push_back(radio);
	auto warnings = CheckAccessibility(form);
	bool found = false;
	for (auto& w : warnings)
		if (contains(w.message, "not in a group")) found = true;
	REQUIRE_FALSE(found);
}
