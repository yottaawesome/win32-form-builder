#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

// === ParseControlType ===

TEST_CASE("ParseControlType parses all 17 control types", "[parser]")
{
    REQUIRE(ParseControlType("Window")          == ControlType::Window);
    REQUIRE(ParseControlType("Button")          == ControlType::Button);
    REQUIRE(ParseControlType("CheckBox")        == ControlType::CheckBox);
    REQUIRE(ParseControlType("RadioButton")     == ControlType::RadioButton);
    REQUIRE(ParseControlType("Label")           == ControlType::Label);
    REQUIRE(ParseControlType("TextBox")         == ControlType::TextBox);
    REQUIRE(ParseControlType("GroupBox")        == ControlType::GroupBox);
    REQUIRE(ParseControlType("ListBox")         == ControlType::ListBox);
    REQUIRE(ParseControlType("ComboBox")        == ControlType::ComboBox);
    REQUIRE(ParseControlType("ProgressBar")     == ControlType::ProgressBar);
    REQUIRE(ParseControlType("TrackBar")        == ControlType::TrackBar);
    REQUIRE(ParseControlType("DateTimePicker")  == ControlType::DateTimePicker);
    REQUIRE(ParseControlType("TabControl")      == ControlType::TabControl);
    REQUIRE(ParseControlType("ListView")        == ControlType::ListView);
    REQUIRE(ParseControlType("TreeView")        == ControlType::TreeView);
    REQUIRE(ParseControlType("UpDown")          == ControlType::UpDown);
    REQUIRE(ParseControlType("RichEdit")        == ControlType::RichEdit);
}

TEST_CASE("ParseControlType throws for unknown type", "[parser]")
{
    REQUIRE_THROWS_AS(ParseControlType("UnknownWidget"), std::runtime_error);
}

// === ControlTypeName ===

TEST_CASE("ControlTypeName returns correct names for all types", "[serializer]")
{
    REQUIRE(ControlTypeName(ControlType::Window)         == "Window");
    REQUIRE(ControlTypeName(ControlType::Button)         == "Button");
    REQUIRE(ControlTypeName(ControlType::CheckBox)       == "CheckBox");
    REQUIRE(ControlTypeName(ControlType::RadioButton)    == "RadioButton");
    REQUIRE(ControlTypeName(ControlType::Label)          == "Label");
    REQUIRE(ControlTypeName(ControlType::TextBox)        == "TextBox");
    REQUIRE(ControlTypeName(ControlType::GroupBox)       == "GroupBox");
    REQUIRE(ControlTypeName(ControlType::ListBox)        == "ListBox");
    REQUIRE(ControlTypeName(ControlType::ComboBox)       == "ComboBox");
    REQUIRE(ControlTypeName(ControlType::ProgressBar)    == "ProgressBar");
    REQUIRE(ControlTypeName(ControlType::TrackBar)       == "TrackBar");
    REQUIRE(ControlTypeName(ControlType::DateTimePicker) == "DateTimePicker");
    REQUIRE(ControlTypeName(ControlType::TabControl)     == "TabControl");
    REQUIRE(ControlTypeName(ControlType::ListView)       == "ListView");
    REQUIRE(ControlTypeName(ControlType::TreeView)       == "TreeView");
    REQUIRE(ControlTypeName(ControlType::UpDown)         == "UpDown");
    REQUIRE(ControlTypeName(ControlType::RichEdit)       == "RichEdit");
}

// === ParseControlType / ControlTypeName roundtrip ===

TEST_CASE("ParseControlType and ControlTypeName are inverse operations", "[parser][serializer]")
{
    auto types = {
        ControlType::Window, ControlType::Button, ControlType::CheckBox,
        ControlType::RadioButton, ControlType::Label, ControlType::TextBox,
        ControlType::GroupBox, ControlType::ListBox, ControlType::ComboBox,
        ControlType::ProgressBar, ControlType::TrackBar, ControlType::DateTimePicker,
        ControlType::TabControl, ControlType::ListView, ControlType::TreeView,
        ControlType::UpDown, ControlType::RichEdit,
    };

    for (auto type : types)
    {
        auto name = ControlTypeName(type);
        auto parsed = ParseControlType(name);
        REQUIRE(parsed == type);
    }
}

// === ParseForm ===

TEST_CASE("ParseForm parses minimal form JSON", "[parser]")
{
    auto json = R"({
        "title": "Test Form",
        "width": 800,
        "height": 600,
        "controls": []
    })";

    auto form = ParseForm(json);
    REQUIRE(form.title == L"Test Form");
    REQUIRE(form.width == 800);
    REQUIRE(form.height == 600);
    REQUIRE(form.controls.empty());
    REQUIRE(form.backgroundColor == -1);
}

TEST_CASE("ParseForm parses form with background color", "[parser]")
{
    auto json = R"({
        "title": "Colored",
        "width": 400,
        "height": 300,
        "backgroundColor": "#FF8040",
        "controls": []
    })";

    auto form = ParseForm(json);
    REQUIRE(form.backgroundColor != -1);

    // #FF8040 → R=0xFF, G=0x80, B=0x40 → COLORREF = 0x004080FF
    auto cr = static_cast<unsigned int>(form.backgroundColor);
    REQUIRE((cr & 0xFF) == 0xFF);
    REQUIRE(((cr >> 8) & 0xFF) == 0x80);
    REQUIRE(((cr >> 16) & 0xFF) == 0x40);
}

TEST_CASE("ParseForm parses controls with all fields", "[parser]")
{
    auto json = R"({
        "title": "Full",
        "width": 640,
        "height": 480,
        "controls": [
            {
                "type": "Button",
                "text": "Click Me",
                "rect": [10, 20, 100, 30],
                "id": 101,
                "onClick": "handleClick",
                "tabIndex": 2
            },
            {
                "type": "TextBox",
                "text": "",
                "rect": [10, 60, 200, 25],
                "id": 102,
                "onChange": "handleChange",
                "onFocus": "handleFocus",
                "onBlur": "handleBlur"
            }
        ]
    })";

    auto form = ParseForm(json);
    REQUIRE(form.controls.size() == 2);

    auto& btn = form.controls[0];
    REQUIRE(btn.type == ControlType::Button);
    REQUIRE(btn.text == L"Click Me");
    REQUIRE(btn.rect.x == 10);
    REQUIRE(btn.rect.y == 20);
    REQUIRE(btn.rect.width == 100);
    REQUIRE(btn.rect.height == 30);
    REQUIRE(btn.id == 101);
    REQUIRE(btn.onClick == "handleClick");
    REQUIRE(btn.tabIndex == 2);

    auto& txt = form.controls[1];
    REQUIRE(txt.type == ControlType::TextBox);
    REQUIRE(txt.id == 102);
    REQUIRE(txt.onChange == "handleChange");
    REQUIRE(txt.onFocus == "handleFocus");
    REQUIRE(txt.onBlur == "handleBlur");
}

TEST_CASE("ParseForm parses all event types", "[parser]")
{
    auto json = R"({
        "title": "Events",
        "width": 640,
        "height": 480,
        "controls": [
            {
                "type": "Button",
                "rect": [0, 0, 100, 25],
                "onClick": "click_handler",
                "onChange": "change_handler",
                "onDoubleClick": "dblclick_handler",
                "onSelectionChange": "selchange_handler",
                "onFocus": "focus_handler",
                "onBlur": "blur_handler",
                "onCheck": "check_handler"
            }
        ]
    })";

    auto form = ParseForm(json);
    auto& c = form.controls[0];
    REQUIRE(c.onClick == "click_handler");
    REQUIRE(c.onChange == "change_handler");
    REQUIRE(c.onDoubleClick == "dblclick_handler");
    REQUIRE(c.onSelectionChange == "selchange_handler");
    REQUIRE(c.onFocus == "focus_handler");
    REQUIRE(c.onBlur == "blur_handler");
    REQUIRE(c.onCheck == "check_handler");
}

// === SerializeForm ===

TEST_CASE("SerializeForm produces valid JSON", "[serializer]")
{
    Form form;
    form.title = L"My Form";
    form.width = 800;
    form.height = 600;

    auto json = SerializeForm(form);
    REQUIRE(!json.empty());

    // Should round-trip back through parser.
    auto parsed = ParseForm(json);
    REQUIRE(parsed.title == L"My Form");
    REQUIRE(parsed.width == 800);
    REQUIRE(parsed.height == 600);
}

TEST_CASE("SerializeForm includes controls", "[serializer]")
{
    Form form;
    form.title = L"With Controls";
    form.width = 640;
    form.height = 480;

    Control btn;
    btn.type = ControlType::Button;
    btn.text = L"OK";
    btn.rect = { 10, 20, 80, 30 };
    btn.id = 1;
    btn.onClick = "doOK";
    btn.tabIndex = 3;
    form.controls.push_back(btn);

    auto json = SerializeForm(form);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.controls.size() == 1);
    REQUIRE(parsed.controls[0].type == ControlType::Button);
    REQUIRE(parsed.controls[0].text == L"OK");
    REQUIRE(parsed.controls[0].rect.x == 10);
    REQUIRE(parsed.controls[0].id == 1);
    REQUIRE(parsed.controls[0].onClick == "doOK");
    REQUIRE(parsed.controls[0].tabIndex == 3);
}

TEST_CASE("SerializeForm handles background color roundtrip", "[serializer]")
{
    Form form;
    form.backgroundColor = 0x004080FF;  // R=FF, G=80, B=40

    auto json = SerializeForm(form);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.backgroundColor == form.backgroundColor);
}

TEST_CASE("SerializeForm omits default-valued optional fields", "[serializer]")
{
    Form form;
    Control c;
    c.type = ControlType::Label;
    c.text = L"Hello";
    c.rect = { 0, 0, 100, 25 };
    form.controls.push_back(c);

    auto json = SerializeForm(form);

    // Fields with default values should not appear in JSON.
    REQUIRE(json.find("onClick") == std::string::npos);
    REQUIRE(json.find("onChange") == std::string::npos);
    REQUIRE(json.find("tabIndex") == std::string::npos);
    REQUIRE(json.find("style") == std::string::npos);
    REQUIRE(json.find("exStyle") == std::string::npos);
}

// === Full roundtrip ===

TEST_CASE("Full form roundtrip preserves all data", "[parser][serializer]")
{
    Form original;
    original.title = L"Roundtrip Test";
    original.width = 1024;
    original.height = 768;
    original.backgroundColor = 0x00C0FFEE;

    Control btn;
    btn.type = ControlType::Button;
    btn.text = L"Submit";
    btn.rect = { 50, 100, 120, 35 };
    btn.id = 10;
    btn.onClick = "onSubmit";
    btn.tabIndex = 1;
    original.controls.push_back(btn);

    Control chk;
    chk.type = ControlType::CheckBox;
    chk.text = L"Agree";
    chk.rect = { 50, 150, 100, 25 };
    chk.id = 11;
    chk.onCheck = "onAgree";
    chk.tabIndex = 2;
    original.controls.push_back(chk);

    auto json = SerializeForm(original);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.title == original.title);
    REQUIRE(parsed.width == original.width);
    REQUIRE(parsed.height == original.height);
    REQUIRE(parsed.backgroundColor == original.backgroundColor);
    REQUIRE(parsed.controls.size() == 2);

    REQUIRE(parsed.controls[0].type == ControlType::Button);
    REQUIRE(parsed.controls[0].text == L"Submit");
    REQUIRE(parsed.controls[0].rect.x == 50);
    REQUIRE(parsed.controls[0].rect.y == 100);
    REQUIRE(parsed.controls[0].rect.width == 120);
    REQUIRE(parsed.controls[0].rect.height == 35);
    REQUIRE(parsed.controls[0].id == 10);
    REQUIRE(parsed.controls[0].onClick == "onSubmit");
    REQUIRE(parsed.controls[0].tabIndex == 1);

    REQUIRE(parsed.controls[1].type == ControlType::CheckBox);
    REQUIRE(parsed.controls[1].text == L"Agree");
    REQUIRE(parsed.controls[1].onCheck == "onAgree");
    REQUIRE(parsed.controls[1].tabIndex == 2);
}

// === File I/O ===

TEST_CASE("SaveFormToFile and LoadFormFromFile roundtrip", "[parser][serializer]")
{
    auto tempPath = std::filesystem::temp_directory_path() / "formbuilder_test_form.json";

    Form form;
    form.title = L"File Test";
    form.width = 320;
    form.height = 240;

    Control lbl;
    lbl.type = ControlType::Label;
    lbl.text = L"Name:";
    lbl.rect = { 10, 10, 60, 20 };
    lbl.id = 1;
    form.controls.push_back(lbl);

    SaveFormToFile(form, tempPath);
    REQUIRE(std::filesystem::exists(tempPath));

    auto loaded = LoadFormFromFile(tempPath);
    REQUIRE(loaded.title == L"File Test");
    REQUIRE(loaded.width == 320);
    REQUIRE(loaded.controls.size() == 1);
    REQUIRE(loaded.controls[0].type == ControlType::Label);
    REQUIRE(loaded.controls[0].text == L"Name:");

    std::filesystem::remove(tempPath);
}

TEST_CASE("LoadFormFromFile throws for non-existent file", "[parser]")
{
    REQUIRE_THROWS_AS(
        LoadFormFromFile("nonexistent_file_that_does_not_exist.json"),
        std::runtime_error);
}
