#include <catch2/catch_test_macros.hpp>
import formbuilder;

using namespace FormDesigner;

// ===================================================================
// Schema
// ===================================================================

TEST_CASE("Control value defaults to 0", "[value]")
{
	Control c;
	REQUIRE(c.value == 0);
}

TEST_CASE("SupportsValue returns true for ProgressBar", "[value]")
{
	REQUIRE(SupportsValue(ControlType::ProgressBar));
}

TEST_CASE("SupportsValue returns true for TrackBar", "[value]")
{
	REQUIRE(SupportsValue(ControlType::TrackBar));
}

TEST_CASE("SupportsValue returns true for UpDown", "[value]")
{
	REQUIRE(SupportsValue(ControlType::UpDown));
}

TEST_CASE("SupportsValue returns false for non-numeric types", "[value]")
{
	REQUIRE_FALSE(SupportsValue(ControlType::Button));
	REQUIRE_FALSE(SupportsValue(ControlType::TextBox));
	REQUIRE_FALSE(SupportsValue(ControlType::Label));
	REQUIRE_FALSE(SupportsValue(ControlType::ComboBox));
	REQUIRE_FALSE(SupportsValue(ControlType::CheckBox));
	REQUIRE_FALSE(SupportsValue(ControlType::Picture));
}

// ===================================================================
// Round-trip (parse → serialize → parse)
// ===================================================================

TEST_CASE("Value round-trips through JSON", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::ProgressBar;
	c.rect = {10, 20, 200, 14};
	c.id = 1;
	c.value = 75;
	form.controls.push_back(c);

	auto json = SerializeForm(form);
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls[0].value == 75);
}

TEST_CASE("Value 0 is omitted from JSON", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::ProgressBar;
	c.rect = {10, 20, 200, 14};
	c.id = 1;
	c.value = 0;
	form.controls.push_back(c);

	auto json = SerializeForm(form);
	REQUIRE(json.find("\"value\"") == std::string::npos);
}

TEST_CASE("Non-zero value is present in JSON", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::TrackBar;
	c.rect = {10, 20, 200, 30};
	c.id = 1;
	c.value = 50;
	form.controls.push_back(c);

	auto json = SerializeForm(form);
	REQUIRE(json.find("\"value\"") != std::string::npos);
}

TEST_CASE("Value round-trips for TrackBar", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::TrackBar;
	c.rect = {0, 0, 150, 30};
	c.id = 2;
	c.value = 42;
	form.controls.push_back(c);

	auto json = SerializeForm(form);
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls[0].value == 42);
}

TEST_CASE("Value round-trips for UpDown", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::UpDown;
	c.rect = {0, 0, 20, 24};
	c.id = 3;
	c.value = 10;
	form.controls.push_back(c);

	auto json = SerializeForm(form);
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls[0].value == 10);
}

// ===================================================================
// Codegen
// ===================================================================

static bool contains(const std::string& haystack, const std::string& needle)
{
	return haystack.find(needle) != std::string::npos;
}

TEST_CASE("Codegen emits PBM_SETPOS for ProgressBar value", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::ProgressBar;
	c.rect = {10, 20, 200, 14};
	c.id = 1;
	c.value = 65;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "PBM_SETPOS"));
	REQUIRE(contains(code, "65"));
}

TEST_CASE("Codegen emits TBM_SETPOS for TrackBar value", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::TrackBar;
	c.rect = {10, 20, 200, 30};
	c.id = 1;
	c.value = 30;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "TBM_SETPOS"));
	REQUIRE(contains(code, "30"));
}

TEST_CASE("Codegen emits UDM_SETPOS32 for UpDown value", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::UpDown;
	c.rect = {10, 20, 20, 24};
	c.id = 1;
	c.value = 5;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "UDM_SETPOS32"));
	REQUIRE(contains(code, "5"));
}

TEST_CASE("Codegen omits value messages when value is 0", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::ProgressBar;
	c.rect = {10, 20, 200, 14};
	c.id = 1;
	c.value = 0;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "PBM_SETPOS"));
}

TEST_CASE("Codegen emits PBM_SETRANGE32 for ProgressBar range", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::ProgressBar;
	c.rect = {10, 20, 200, 14};
	c.id = 1;
	c.validation.min = 0;
	c.validation.max = 200;
	c.value = 100;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "PBM_SETRANGE32"));
	REQUIRE(contains(code, "PBM_SETPOS"));
}

TEST_CASE("Codegen emits TBM_SETRANGEMIN/MAX for TrackBar range", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::TrackBar;
	c.rect = {10, 20, 200, 30};
	c.id = 1;
	c.validation.min = 10;
	c.validation.max = 90;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "TBM_SETRANGEMIN"));
	REQUIRE(contains(code, "TBM_SETRANGEMAX"));
}

TEST_CASE("Codegen emits UDM_SETRANGE32 for UpDown range", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::UpDown;
	c.rect = {10, 20, 20, 24};
	c.id = 1;
	c.validation.min = 1;
	c.validation.max = 100;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "UDM_SETRANGE32"));
}

TEST_CASE("Codegen omits range when min and max are both 0", "[value]")
{
	Form form;
	Control c;
	c.type = ControlType::ProgressBar;
	c.rect = {10, 20, 200, 14};
	c.id = 1;
	c.validation.min = 0;
	c.validation.max = 0;
	form.controls.push_back(c);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "PBM_SETRANGE32"));
}
