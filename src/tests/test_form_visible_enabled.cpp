#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static auto contains(const std::string& haystack, const std::string& needle) -> bool
{
	return haystack.find(needle) != std::string::npos;
}

// === Schema defaults ===

TEST_CASE("Form visible defaults to true", "[form-visible-enabled]")
{
	auto form = Form{};
	REQUIRE(form.visible == true);
}

TEST_CASE("Form enabled defaults to true", "[form-visible-enabled]")
{
	auto form = Form{};
	REQUIRE(form.enabled == true);
}

// === Parser ===

TEST_CASE("Parse form visible=false from JSON", "[form-visible-enabled]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"visible": false,
		"controls": []
	})");

	auto form = ParseForm(json);
	REQUIRE(form.visible == false);
}

TEST_CASE("Parse form enabled=false from JSON", "[form-visible-enabled]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"enabled": false,
		"controls": []
	})");

	auto form = ParseForm(json);
	REQUIRE(form.enabled == false);
}

TEST_CASE("Omitted form visible/enabled default to true", "[form-visible-enabled]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": []
	})");

	auto form = ParseForm(json);
	REQUIRE(form.visible == true);
	REQUIRE(form.enabled == true);
}

// === Serializer ===

TEST_CASE("Serialize form visible=false", "[form-visible-enabled]")
{
	auto form = Form{};
	form.visible = false;
	auto output = SerializeForm(form);
	REQUIRE(contains(output, "\"visible\""));
	REQUIRE(contains(output, "false"));
}

TEST_CASE("Serialize form enabled=false", "[form-visible-enabled]")
{
	auto form = Form{};
	form.enabled = false;
	auto output = SerializeForm(form);
	REQUIRE(contains(output, "\"enabled\""));
	REQUIRE(contains(output, "false"));
}

TEST_CASE("Form visible=true is not serialized (default)", "[form-visible-enabled]")
{
	auto form = Form{};
	auto output = SerializeForm(form);
	REQUIRE_FALSE(contains(output, "\"visible\""));
}

TEST_CASE("Form enabled=true is not serialized (default)", "[form-visible-enabled]")
{
	auto form = Form{};
	auto output = SerializeForm(form);
	REQUIRE_FALSE(contains(output, "\"enabled\""));
}

// === Round-trip ===

TEST_CASE("Round-trip preserves form visible=false", "[form-visible-enabled]")
{
	auto form = Form{};
	form.visible = false;
	auto output = SerializeForm(form);
	auto reparsed = ParseForm(output);
	REQUIRE(reparsed.visible == false);
}

TEST_CASE("Round-trip preserves form enabled=false", "[form-visible-enabled]")
{
	auto form = Form{};
	form.enabled = false;
	auto output = SerializeForm(form);
	auto reparsed = ParseForm(output);
	REQUIRE(reparsed.enabled == false);
}

// === Codegen ===

TEST_CASE("Codegen emits ShowWindow for visible form", "[form-visible-enabled]")
{
	auto form = Form{};
	form.visible = true;
	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "ShowWindow(hwnd, nCmdShow)"));
}

TEST_CASE("Codegen omits ShowWindow for hidden form", "[form-visible-enabled]")
{
	auto form = Form{};
	form.visible = false;
	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "ShowWindow(hwnd, nCmdShow)"));
	REQUIRE(contains(code, "Form starts hidden"));
}

TEST_CASE("Codegen emits WS_DISABLED for disabled form", "[form-visible-enabled]")
{
	auto form = Form{};
	form.enabled = false;
	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "WS_DISABLED"));
}

TEST_CASE("Codegen omits WS_DISABLED for enabled form", "[form-visible-enabled]")
{
	auto form = Form{};
	form.enabled = true;
	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "WS_DISABLED"));
}

// === RC Export ===

TEST_CASE("RC export includes WS_DISABLED for disabled form", "[form-visible-enabled]")
{
	auto form = Form{};
	form.enabled = false;
	auto rc = GenerateRcDialog(form);
	// WS_DISABLED = 0x08000000 should be ORed into STYLE
	auto style = std::string("STYLE 0x");
	auto pos = rc.find(style);
	REQUIRE(pos != std::string::npos);
	auto hexStr = rc.substr(pos + 6, 10);
	auto styleValue = std::stoul(hexStr, nullptr, 16);
	REQUIRE((styleValue & 0x08000000) != 0);
}

TEST_CASE("RC export omits WS_DISABLED for enabled form", "[form-visible-enabled]")
{
	auto form = Form{};
	form.enabled = true;
	auto rc = GenerateRcDialog(form);
	auto style = std::string("STYLE 0x");
	auto pos = rc.find(style);
	REQUIRE(pos != std::string::npos);
	auto hexStr = rc.substr(pos + 6, 10);
	auto styleValue = std::stoul(hexStr, nullptr, 16);
	REQUIRE((styleValue & 0x08000000) == 0);
}

// === Both hidden and disabled ===

TEST_CASE("Form can be both hidden and disabled", "[form-visible-enabled]")
{
	auto form = Form{};
	form.visible = false;
	form.enabled = false;

	auto output = SerializeForm(form);
	REQUIRE(contains(output, "\"visible\""));
	REQUIRE(contains(output, "\"enabled\""));

	auto reparsed = ParseForm(output);
	REQUIRE(reparsed.visible == false);
	REQUIRE(reparsed.enabled == false);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "WS_DISABLED"));
	REQUIRE_FALSE(contains(code, "ShowWindow(hwnd, nCmdShow)"));
}
