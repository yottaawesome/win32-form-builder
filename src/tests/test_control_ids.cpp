#include <catch2/catch_test_macros.hpp>
import formbuilder;

using namespace FormDesigner;

static auto MakeControl(ControlType type, int id, const std::wstring& text = L"") -> Control
{
	auto c = Control{};
	c.type = type;
	c.id = id;
	c.text = text;
	return c;
}

TEST_CASE("GenerateControlIds produces header guard and namespace", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Button, 101, L"OK"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("#pragma once") != std::string::npos);
	REQUIRE(result.find("namespace Controls") != std::string::npos);
	REQUIRE(result.find("inline constexpr int") != std::string::npos);
}

TEST_CASE("GenerateControlIds derives PascalCase name from text", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Button, 101, L"OK"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("OkButton = 101") != std::string::npos);
}

TEST_CASE("GenerateControlIds strips accelerator prefix", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Button, 101, L"&OK"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("OkButton = 101") != std::string::npos);
}

TEST_CASE("GenerateControlIds handles multi-word text", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::TextBox, 102, L"User &Name:"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("UserNameText = 102") != std::string::npos);
}

TEST_CASE("GenerateControlIds uses type suffix for different control types", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::CheckBox, 201, L"Agree"));
	form.controls.push_back(MakeControl(ControlType::RadioButton, 202, L"Option A"));
	form.controls.push_back(MakeControl(ControlType::Label, 203, L"Status"));
	form.controls.push_back(MakeControl(ControlType::ComboBox, 204, L"Country"));
	form.controls.push_back(MakeControl(ControlType::ListBox, 205, L"Items"));
	form.controls.push_back(MakeControl(ControlType::ProgressBar, 206, L"Loading"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("AgreeCheck = 201") != std::string::npos);
	REQUIRE(result.find("OptionARadio = 202") != std::string::npos);
	REQUIRE(result.find("StatusLabel = 203") != std::string::npos);
	REQUIRE(result.find("CountryCombo = 204") != std::string::npos);
	REQUIRE(result.find("ItemsList = 205") != std::string::npos);
	REQUIRE(result.find("LoadingProgress = 206") != std::string::npos);
}

TEST_CASE("GenerateControlIds falls back to type+ID for empty text", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Button, 301, L""));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("Button301 = 301") != std::string::npos);
}

TEST_CASE("GenerateControlIds skips controls with id=0", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Label, 0, L"Static"));
	form.controls.push_back(MakeControl(ControlType::Button, 101, L"OK"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("StaticLabel") == std::string::npos);
	REQUIRE(result.find("OkButton = 101") != std::string::npos);
}

TEST_CASE("GenerateControlIds disambiguates duplicate names by appending ID", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Button, 101, L"OK"));
	form.controls.push_back(MakeControl(ControlType::Button, 102, L"OK"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("OkButton101 = 101") != std::string::npos);
	REQUIRE(result.find("OkButton102 = 102") != std::string::npos);
}

TEST_CASE("GenerateControlIds strips special characters", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Button, 101, L"Click (Here)!"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("ClickHereButton = 101") != std::string::npos);
}

TEST_CASE("GenerateControlIds handles hyphenated text", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::TextBox, 101, L"first-name"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("FirstNameText = 101") != std::string::npos);
}

TEST_CASE("GenerateControlIds handles multiple controls", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Label, 201, L"Name:"));
	form.controls.push_back(MakeControl(ControlType::TextBox, 202, L"Name"));
	form.controls.push_back(MakeControl(ControlType::Button, 203, L"&Submit"));
	form.controls.push_back(MakeControl(ControlType::Button, 204, L"&Cancel"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("NameLabel = 201") != std::string::npos);
	REQUIRE(result.find("NameText = 202") != std::string::npos);
	REQUIRE(result.find("SubmitButton = 203") != std::string::npos);
	REQUIRE(result.find("CancelButton = 204") != std::string::npos);
}

TEST_CASE("GenerateControlIds includes comment header", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::Button, 101, L"OK"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("Control ID constants generated by Win32 Form Builder") != std::string::npos);
	REQUIRE(result.find("FormWindow") != std::string::npos);
}

TEST_CASE("GenerateControlIds handles all wrapper types", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::TrackBar, 101, L"Volume"));
	form.controls.push_back(MakeControl(ControlType::UpDown, 102, L"Count"));
	form.controls.push_back(MakeControl(ControlType::DateTimePicker, 103, L"Start"));
	form.controls.push_back(MakeControl(ControlType::RichEdit, 104, L"Notes"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("VolumeTrack = 101") != std::string::npos);
	REQUIRE(result.find("CountUpDown = 102") != std::string::npos);
	REQUIRE(result.find("StartDateTime = 103") != std::string::npos);
	REQUIRE(result.find("NotesRichEdit = 104") != std::string::npos);
}

TEST_CASE("GenerateControlIds handles remaining control types", "[controlids]")
{
	auto form = Form{};
	form.controls.push_back(MakeControl(ControlType::GroupBox, 101, L"Options"));
	form.controls.push_back(MakeControl(ControlType::TabControl, 102, L"Pages"));
	form.controls.push_back(MakeControl(ControlType::ListView, 103, L"Files"));
	form.controls.push_back(MakeControl(ControlType::TreeView, 104, L"Tree"));
	form.controls.push_back(MakeControl(ControlType::MonthCalendar, 105, L"Date"));
	form.controls.push_back(MakeControl(ControlType::Link, 106, L"Help"));
	form.controls.push_back(MakeControl(ControlType::IPAddress, 107, L"Server"));
	form.controls.push_back(MakeControl(ControlType::HotKey, 108, L"Shortcut"));
	form.controls.push_back(MakeControl(ControlType::Picture, 109, L"Logo"));
	form.controls.push_back(MakeControl(ControlType::Separator, 110, L"Divider"));
	form.controls.push_back(MakeControl(ControlType::Animation, 111, L"Spinner"));

	auto result = GenerateControlIds(form);

	REQUIRE(result.find("OptionsGroup = 101") != std::string::npos);
	REQUIRE(result.find("PagesTab = 102") != std::string::npos);
	REQUIRE(result.find("FilesListView = 103") != std::string::npos);
	REQUIRE(result.find("TreeTreeView = 104") != std::string::npos);
	REQUIRE(result.find("DateCalendar = 105") != std::string::npos);
	REQUIRE(result.find("HelpLink = 106") != std::string::npos);
	REQUIRE(result.find("ServerIpAddress = 107") != std::string::npos);
	REQUIRE(result.find("ShortcutHotKey = 108") != std::string::npos);
	REQUIRE(result.find("LogoPicture = 109") != std::string::npos);
	REQUIRE(result.find("DividerSeparator = 110") != std::string::npos);
	REQUIRE(result.find("SpinnerAnimation = 111") != std::string::npos);
}
