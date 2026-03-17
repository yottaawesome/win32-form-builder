#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static auto contains(const std::string& haystack, const std::string& needle) -> bool
{
	return haystack.find(needle) != std::string::npos;
}

// === Schema defaults ===

TEST_CASE("Control imagePath defaults to empty", "[picture]")
{
	auto ctrl = Control{};
	REQUIRE(ctrl.imagePath.empty());
}

// === ImageTypeFromPath helper ===

TEST_CASE("ImageTypeFromPath returns 1 for .bmp", "[picture]")
{
	REQUIRE(ImageTypeFromPath(L"icon.bmp") == 1);
	REQUIRE(ImageTypeFromPath(L"C:\\images\\photo.BMP") == 1);
}

TEST_CASE("ImageTypeFromPath returns 2 for .ico", "[picture]")
{
	REQUIRE(ImageTypeFromPath(L"app.ico") == 2);
	REQUIRE(ImageTypeFromPath(L"path\\to\\icon.ICO") == 2);
}

TEST_CASE("ImageTypeFromPath returns 0 for unknown", "[picture]")
{
	REQUIRE(ImageTypeFromPath(L"photo.png") == 0);
	REQUIRE(ImageTypeFromPath(L"doc.txt") == 0);
	REQUIRE(ImageTypeFromPath(L"ab") == 0);
	REQUIRE(ImageTypeFromPath(L"") == 0);
}

// === Parser ===

TEST_CASE("Parse imagePath from JSON", "[picture]")
{
	auto json = std::string(R"({
		"title": "Test", "width": 400, "height": 300,
		"controls": [{
			"type": "Picture",
			"text": "",
			"rect": [10, 10, 100, 80],
			"imagePath": "images/logo.bmp"
		}]
	})");

	auto form = ParseForm(json);
	REQUIRE(form.controls.size() == 1);
	REQUIRE(form.controls[0].imagePath == L"images/logo.bmp");
}

TEST_CASE("Parse Picture without imagePath keeps empty", "[picture]")
{
	auto json = std::string(R"({
		"title": "Test", "width": 400, "height": 300,
		"controls": [{
			"type": "Picture",
			"text": "",
			"rect": [10, 10, 100, 80]
		}]
	})");

	auto form = ParseForm(json);
	REQUIRE(form.controls[0].imagePath.empty());
}

// === Serializer ===

TEST_CASE("imagePath omitted from JSON when empty", "[picture]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::Picture;
	ctrl.rect = { 10, 10, 100, 80 };
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	REQUIRE_FALSE(contains(json, "imagePath"));
}

TEST_CASE("imagePath serialized to JSON when set", "[picture]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::Picture;
	ctrl.rect = { 10, 10, 100, 80 };
	ctrl.imagePath = L"images/logo.bmp";
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	REQUIRE(contains(json, "\"imagePath\""));
	REQUIRE(contains(json, "images/logo.bmp"));
}

// === Round-trip ===

TEST_CASE("imagePath round-trips through serialize/parse", "[picture]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::Picture;
	ctrl.rect = { 10, 10, 100, 80 };
	ctrl.imagePath = L"assets\\icon.ico";
	form.controls.push_back(ctrl);

	auto json = SerializeForm(form);
	auto parsed = ParseForm(json);
	REQUIRE(parsed.controls.size() == 1);
	REQUIRE(parsed.controls[0].imagePath == L"assets\\icon.ico");
}

// === Codegen ===

TEST_CASE("Codegen emits LoadImageW for Picture with imagePath", "[picture][codegen]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::Picture;
	ctrl.rect = { 10, 10, 100, 80 };
	ctrl.imagePath = L"logo.bmp";
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "LoadImageW"));
	REQUIRE(contains(code, "L\"logo.bmp\""));
	REQUIRE(contains(code, "IMAGE_BITMAP"));
	REQUIRE(contains(code, "STM_SETIMAGE"));
	REQUIRE(contains(code, "SS_BITMAP"));
}

TEST_CASE("Codegen emits IMAGE_ICON for .ico files", "[picture][codegen]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::Picture;
	ctrl.rect = { 10, 10, 32, 32 };
	ctrl.imagePath = L"app.ico";
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE(contains(code, "IMAGE_ICON"));
	REQUIRE(contains(code, "SS_ICON"));
}

TEST_CASE("Codegen omits LoadImageW when imagePath empty", "[picture][codegen]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::Picture;
	ctrl.rect = { 10, 10, 100, 80 };
	form.controls.push_back(ctrl);

	auto code = GenerateCode(form, false);
	REQUIRE_FALSE(contains(code, "LoadImageW"));
	REQUIRE_FALSE(contains(code, "STM_SETIMAGE"));
}

// === RC export ===

TEST_CASE("RC export emits image comment for Picture with imagePath", "[picture][rc]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::Picture;
	ctrl.rect = { 10, 10, 100, 80 };
	ctrl.imagePath = L"images/logo.bmp";
	form.controls.push_back(ctrl);

	auto rc = GenerateRcDialog(form);
	REQUIRE(contains(rc, "// Image: images/logo.bmp"));
}

TEST_CASE("RC export no image comment when imagePath empty", "[picture][rc]")
{
	auto form = Form{ .title = L"Test", .width = 400, .height = 300 };
	auto ctrl = Control{};
	ctrl.type = ControlType::Picture;
	ctrl.rect = { 10, 10, 100, 80 };
	form.controls.push_back(ctrl);

	auto rc = GenerateRcDialog(form);
	REQUIRE_FALSE(contains(rc, "// Image:"));
}
