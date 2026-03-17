#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static auto contains(const std::string& haystack, const std::string& needle) -> bool
{
	return haystack.find(needle) != std::string::npos;
}

// === Schema defaults ===

TEST_CASE("Control visible defaults to true", "[visible]")
{
	auto ctrl = Control{};
	REQUIRE(ctrl.visible == true);
}

// === Parser ===

TEST_CASE("ParseControl reads visible=false from JSON", "[visible][parser]")
{
	auto json = R"({"type":"Button","text":"OK","rect":[10,10,80,25],"visible":false})";
	auto ctrl = ParseControl(nlohmann::json::parse(json));
	REQUIRE(ctrl.visible == false);
}

TEST_CASE("ParseControl defaults visible to true when absent", "[visible][parser]")
{
	auto json = R"({"type":"Button","text":"OK","rect":[10,10,80,25]})";
	auto ctrl = ParseControl(nlohmann::json::parse(json));
	REQUIRE(ctrl.visible == true);
}

TEST_CASE("ParseControl reads visible=true from JSON", "[visible][parser]")
{
	auto json = R"({"type":"Label","text":"Hello","rect":[0,0,50,20],"visible":true})";
	auto ctrl = ParseControl(nlohmann::json::parse(json));
	REQUIRE(ctrl.visible == true);
}

// === Serializer ===

TEST_CASE("SerializeControl omits visible when true", "[visible][serializer]")
{
	auto ctrl = Control{ .type = ControlType::Button, .text = L"OK" };
	ctrl.visible = true;
	auto j = SerializeControl(ctrl);
	REQUIRE_FALSE(j.contains("visible"));
}

TEST_CASE("SerializeControl writes visible=false", "[visible][serializer]")
{
	auto ctrl = Control{ .type = ControlType::Button, .text = L"OK" };
	ctrl.visible = false;
	auto j = SerializeControl(ctrl);
	REQUIRE(j.contains("visible"));
	REQUIRE(j["visible"].get<bool>() == false);
}

// === Round-trip ===

TEST_CASE("Visible property round-trips through serialize/parse", "[visible][roundtrip]")
{
	auto original = Control{ .type = ControlType::TextBox, .text = L"Input" };
	original.visible = false;

	auto j = SerializeControl(original);
	auto parsed = ParseControl(j);
	REQUIRE(parsed.visible == false);
}

TEST_CASE("Visible=true round-trips correctly", "[visible][roundtrip]")
{
	auto original = Control{ .type = ControlType::Label, .text = L"Test" };
	original.visible = true;

	auto j = SerializeControl(original);
	auto parsed = ParseControl(j);
	REQUIRE(parsed.visible == true);
}

// === Form-level round-trip ===

TEST_CASE("Form with hidden control round-trips through serialize/parse", "[visible][roundtrip]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button, .text = L"Hidden" };
	ctrl.visible = false;
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls.size() == 1);
	REQUIRE(parsed.controls[0].visible == false);
}

// === Code generation ===

TEST_CASE("GenerateCode includes WS_VISIBLE for visible controls", "[visible][codegen]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button, .text = L"OK" };
	ctrl.visible = true;
	ctrl.id = 1;
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "WS_VISIBLE"));
}

TEST_CASE("GenerateCode omits WS_VISIBLE for hidden controls", "[visible][codegen]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button, .text = L"OK" };
	ctrl.visible = false;
	ctrl.id = 1;
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	// The style expression for this control should NOT contain WS_VISIBLE
	// Find the CreateWindowExW call for this control
	auto stylePos = code.find("WS_CHILD");
	REQUIRE(stylePos != std::string::npos);
	// Get a section around the style expression
	auto section = code.substr(stylePos, 200);
	REQUIRE_FALSE(contains(section, "WS_VISIBLE"));
}

// === RC export ===

TEST_CASE("RC export includes NOT WS_VISIBLE for hidden control", "[visible][rc]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button, .text = L"OK" };
	ctrl.visible = false;
	ctrl.id = 1;
	form.controls.push_back(ctrl);

	auto rc = GenerateRcDialog(form);
	REQUIRE(contains(rc, "NOT WS_VISIBLE"));
}

TEST_CASE("RC export does not add NOT WS_VISIBLE for visible control", "[visible][rc]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::Button, .text = L"OK" };
	ctrl.visible = true;
	ctrl.id = 1;
	form.controls.push_back(ctrl);

	auto rc = GenerateRcDialog(form);
	REQUIRE_FALSE(contains(rc, "NOT WS_VISIBLE"));
}

TEST_CASE("RC generic CONTROL uses non-visible style for hidden control", "[visible][rc]")
{
	auto form = Form{};
	auto ctrl = Control{ .type = ControlType::ProgressBar };
	ctrl.visible = false;
	ctrl.id = 1;
	form.controls.push_back(ctrl);

	auto rc = GenerateRcDialog(form);
	// Generic CONTROL should use style without WS_VISIBLE (0x40010000 instead of 0x50010000)
	REQUIRE(contains(rc, "0x40010000"));
}
