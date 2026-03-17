#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static auto contains(const std::string& haystack, const std::string& needle) -> bool
{
	return haystack.find(needle) != std::string::npos;
}

// === Schema defaults ===

TEST_CASE("ValidationInfo defaults are all unset", "[validation]")
{
	auto v = ValidationInfo{};
	REQUIRE_FALSE(v.required);
	REQUIRE(v.minLength == 0);
	REQUIRE(v.maxLength == 0);
	REQUIRE(v.pattern.empty());
	REQUIRE(v.min == 0);
	REQUIRE(v.max == 0);
}

TEST_CASE("ValidationInfo isSet returns false when default", "[validation]")
{
	auto v = ValidationInfo{};
	REQUIRE_FALSE(v.isSet());
}

TEST_CASE("ValidationInfo isSet returns true for required", "[validation]")
{
	auto v = ValidationInfo{};
	v.required = true;
	REQUIRE(v.isSet());
}

TEST_CASE("ValidationInfo isSet returns true for minLength", "[validation]")
{
	auto v = ValidationInfo{};
	v.minLength = 3;
	REQUIRE(v.isSet());
}

TEST_CASE("ValidationInfo isSet returns true for pattern", "[validation]")
{
	auto v = ValidationInfo{};
	v.pattern = "^[A-Z]+$";
	REQUIRE(v.isSet());
}

TEST_CASE("ValidationInfo isSet returns true for range", "[validation]")
{
	auto v = ValidationInfo{};
	v.min = 10;
	REQUIRE(v.isSet());
}

// === SupportsX helpers ===

TEST_CASE("SupportsTextValidation for TextBox and RichEdit", "[validation]")
{
	REQUIRE(SupportsTextValidation(ControlType::TextBox));
	REQUIRE(SupportsTextValidation(ControlType::RichEdit));
	REQUIRE_FALSE(SupportsTextValidation(ControlType::Button));
	REQUIRE_FALSE(SupportsTextValidation(ControlType::TrackBar));
}

TEST_CASE("SupportsRangeValidation for numeric controls", "[validation]")
{
	REQUIRE(SupportsRangeValidation(ControlType::TrackBar));
	REQUIRE(SupportsRangeValidation(ControlType::UpDown));
	REQUIRE(SupportsRangeValidation(ControlType::ProgressBar));
	REQUIRE_FALSE(SupportsRangeValidation(ControlType::TextBox));
}

TEST_CASE("SupportsRequiredValidation for input controls", "[validation]")
{
	REQUIRE(SupportsRequiredValidation(ControlType::TextBox));
	REQUIRE(SupportsRequiredValidation(ControlType::ComboBox));
	REQUIRE(SupportsRequiredValidation(ControlType::ListBox));
	REQUIRE(SupportsRequiredValidation(ControlType::RichEdit));
	REQUIRE(SupportsRequiredValidation(ControlType::DateTimePicker));
	REQUIRE_FALSE(SupportsRequiredValidation(ControlType::Label));
}

// === Parser round-trip ===

TEST_CASE("Parse validation object from JSON", "[validation]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": [{
			"type": "TextBox",
			"text": "",
			"rect": [10, 10, 100, 25],
			"validation": {
				"required": true,
				"minLength": 3,
				"maxLength": 50,
				"pattern": "^[A-Za-z]+$"
			}
		}]
	})");

	auto form = ParseForm(json);
	REQUIRE(form.controls.size() == 1);

	auto& v = form.controls[0].validation;
	REQUIRE(v.required == true);
	REQUIRE(v.minLength == 3);
	REQUIRE(v.maxLength == 50);
	REQUIRE(v.pattern == "^[A-Za-z]+$");
	REQUIRE(v.min == 0);
	REQUIRE(v.max == 0);
}

TEST_CASE("Parse range validation from JSON", "[validation]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": [{
			"type": "TrackBar",
			"text": "",
			"rect": [10, 10, 200, 30],
			"validation": {
				"min": 0,
				"max": 100
			}
		}]
	})");

	auto form = ParseForm(json);
	auto& v = form.controls[0].validation;
	REQUIRE_FALSE(v.required);
	REQUIRE(v.min == 0);
	REQUIRE(v.max == 100);
}

TEST_CASE("Controls without validation keep defaults", "[validation]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": [{
			"type": "Button",
			"text": "OK",
			"rect": [10, 10, 80, 25]
		}]
	})");

	auto form = ParseForm(json);
	REQUIRE_FALSE(form.controls[0].validation.isSet());
}

// === Serializer ===

TEST_CASE("Validation omitted from JSON when unset", "[validation]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::TextBox;
	ctrl.rect = { 10, 10, 100, 25 };
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	REQUIRE_FALSE(contains(json, "validation"));
}

TEST_CASE("Validation serialized to JSON when set", "[validation]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::TextBox;
	ctrl.rect = { 10, 10, 100, 25 };
	ctrl.validation.required = true;
	ctrl.validation.minLength = 3;
	ctrl.validation.maxLength = 50;
	ctrl.validation.pattern = "^[A-Z]+$";
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	REQUIRE(contains(json, "\"validation\""));
	REQUIRE(contains(json, "\"required\": true"));
	REQUIRE(contains(json, "\"minLength\": 3"));
	REQUIRE(contains(json, "\"maxLength\": 50"));
	REQUIRE(contains(json, "\"pattern\": \"^[A-Z]+$\""));
}

TEST_CASE("Validation round-trip preserves all fields", "[validation]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::TextBox;
	ctrl.rect = { 10, 10, 100, 25 };
	ctrl.validation.required = true;
	ctrl.validation.minLength = 2;
	ctrl.validation.maxLength = 100;
	ctrl.validation.pattern = "[0-9]+";
	ctrl.validation.min = 5;
	ctrl.validation.max = 999;
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	auto form2 = ParseForm(json);
	REQUIRE(form2.controls.size() == 1);

	auto& v = form2.controls[0].validation;
	REQUIRE(v.required == true);
	REQUIRE(v.minLength == 2);
	REQUIRE(v.maxLength == 100);
	REQUIRE(v.pattern == "[0-9]+");
	REQUIRE(v.min == 5);
	REQUIRE(v.max == 999);
}

// === Codegen ===

TEST_CASE("Codegen emits validation comment for validated control", "[validation]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::TextBox;
	ctrl.text = L"";
	ctrl.rect = { 10, 10, 100, 25 };
	ctrl.validation.required = true;
	ctrl.validation.minLength = 3;
	ctrl.validation.maxLength = 50;
	ctrl.validation.pattern = "^[A-Za-z]+$";
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "// Validation:"));
	REQUIRE(contains(code, "required"));
	REQUIRE(contains(code, "minLength=3"));
	REQUIRE(contains(code, "maxLength=50"));
	REQUIRE(contains(code, "pattern=\"^[A-Za-z]+$\""));
}

TEST_CASE("Codegen omits validation comment when unset", "[validation]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::TextBox;
	ctrl.text = L"";
	ctrl.rect = { 10, 10, 100, 25 };
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "// Validation:"));
}

TEST_CASE("Codegen emits range validation for TrackBar", "[validation]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::TrackBar;
	ctrl.rect = { 10, 10, 200, 30 };
	ctrl.validation.min = 10;
	ctrl.validation.max = 100;
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "// Validation:"));
	REQUIRE(contains(code, "min=10"));
	REQUIRE(contains(code, "max=100"));
}
