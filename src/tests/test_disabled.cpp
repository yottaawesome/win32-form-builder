#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static auto contains(const std::string& haystack, const std::string& needle) -> bool
{
	return haystack.find(needle) != std::string::npos;
}

// === Schema defaults ===

TEST_CASE("Control enabled defaults to true", "[disabled]")
{
	auto ctrl = Control{};
	REQUIRE(ctrl.enabled == true);
}

// === Parser round-trip ===

TEST_CASE("Parse enabled=false from JSON", "[disabled]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": [
			{ "type": "Button", "text": "OK", "rect": [10, 10, 80, 30], "id": 101, "enabled": false }
		]
	})");

	auto form = ParseForm(json);
	REQUIRE(form.controls.size() == 1);
	REQUIRE(form.controls[0].enabled == false);
}

TEST_CASE("Parse enabled=true (explicit) from JSON", "[disabled]")
{
	auto json = std::string(R"({
		"title": "Test",
		"width": 400,
		"height": 300,
		"controls": [
			{ "type": "Button", "text": "OK", "rect": [10, 10, 80, 30], "id": 101, "enabled": true }
		]
	})");

	auto form = ParseForm(json);
	REQUIRE(form.controls[0].enabled == true);
}

TEST_CASE("Omitted enabled defaults to true", "[disabled]")
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
	REQUIRE(form.controls[0].enabled == true);
}

// === Serializer ===

TEST_CASE("Serialize enabled=false to JSON", "[disabled]")
{
	auto form = Form{};
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 10, 80, 30 };
	btn.id = 101;
	btn.enabled = false;
	form.controls.push_back(btn);

	auto output = SerializeForm(form);
	REQUIRE(contains(output, "\"enabled\""));
	REQUIRE(contains(output, "false"));
}

TEST_CASE("Enabled=true is not serialized (default omission)", "[disabled]")
{
	auto form = Form{};
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 10, 80, 30 };
	btn.id = 101;
	btn.enabled = true;
	form.controls.push_back(btn);

	auto output = SerializeForm(form);
	REQUIRE_FALSE(contains(output, "\"enabled\""));
}

TEST_CASE("Round-trip preserves enabled=false", "[disabled]")
{
	auto form = Form{};
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 10, 80, 30 };
	btn.id = 101;
	btn.enabled = false;
	form.controls.push_back(btn);

	auto output = SerializeForm(form);
	auto reparsed = ParseForm(output);
	REQUIRE(reparsed.controls[0].enabled == false);
}

// === Codegen ===

TEST_CASE("Codegen emits WS_DISABLED for disabled control", "[disabled]")
{
	auto form = Form{};
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 10, 80, 30 };
	btn.id = 101;
	btn.enabled = false;
	form.controls.push_back(btn);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "WS_DISABLED"));
}

TEST_CASE("Codegen omits WS_DISABLED for enabled control", "[disabled]")
{
	auto form = Form{};
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 10, 80, 30 };
	btn.id = 101;
	btn.enabled = true;
	form.controls.push_back(btn);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "WS_DISABLED"));
}

// === RC Export ===

TEST_CASE("RC export includes WS_DISABLED for disabled control", "[disabled]")
{
	auto form = Form{};
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 10, 80, 30 };
	btn.id = 101;
	btn.enabled = false;
	form.controls.push_back(btn);

	auto rc = GenerateRcDialog(form);
	REQUIRE(contains(rc, "WS_DISABLED"));
}

TEST_CASE("RC export omits WS_DISABLED for enabled control", "[disabled]")
{
	auto form = Form{};
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 10, 80, 30 };
	btn.id = 101;
	btn.enabled = true;
	form.controls.push_back(btn);

	auto rc = GenerateRcDialog(form);
	REQUIRE_FALSE(contains(rc, "WS_DISABLED"));
}

TEST_CASE("Disabled and hidden can coexist", "[disabled]")
{
	auto form = Form{};
	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"OK";
	btn.rect = { 10, 10, 80, 30 };
	btn.id = 101;
	btn.visible = false;
	btn.enabled = false;
	form.controls.push_back(btn);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "WS_DISABLED"));
	REQUIRE_FALSE(contains(code, "WS_VISIBLE"));

	auto output = SerializeForm(form);
	REQUIRE(contains(output, "\"visible\""));
	REQUIRE(contains(output, "\"enabled\""));

	auto reparsed = ParseForm(output);
	REQUIRE(reparsed.controls[0].visible == false);
	REQUIRE(reparsed.controls[0].enabled == false);
}
