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

// === textAlign ===

TEST_CASE("ParseControl parses textAlign", "[parser]")
{
    auto json = R"({
        "title": "Align",
        "width": 400,
        "height": 300,
        "controls": [
            { "type": "Label", "rect": [0,0,100,20], "textAlign": "center" },
            { "type": "TextBox", "rect": [0,0,100,20], "textAlign": "right" },
            { "type": "Button", "rect": [0,0,100,25] }
        ]
    })";

    auto form = ParseForm(json);
    REQUIRE(form.controls[0].textAlign == TextAlign::Center);
    REQUIRE(form.controls[1].textAlign == TextAlign::Right);
    REQUIRE(form.controls[2].textAlign == TextAlign::Left); // default
}

TEST_CASE("SerializeForm omits textAlign when left (default)", "[serializer]")
{
    Form form;
    Control c;
    c.type = ControlType::Label;
    c.text = L"Test";
    c.rect = { 0, 0, 100, 20 };
    form.controls.push_back(c);

    auto json = SerializeForm(form);
    REQUIRE(json.find("textAlign") == std::string::npos);
}

TEST_CASE("textAlign roundtrips through serialize/parse", "[parser][serializer]")
{
    Form form;

    Control lbl;
    lbl.type = ControlType::Label;
    lbl.text = L"Centered";
    lbl.rect = { 0, 0, 100, 20 };
    lbl.textAlign = TextAlign::Center;
    form.controls.push_back(lbl);

    Control txt;
    txt.type = ControlType::TextBox;
    txt.rect = { 0, 30, 100, 25 };
    txt.textAlign = TextAlign::Right;
    form.controls.push_back(txt);

    auto json = SerializeForm(form);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.controls[0].textAlign == TextAlign::Center);
    REQUIRE(parsed.controls[1].textAlign == TextAlign::Right);
}

// === Locked field ===

TEST_CASE("ParseControl reads locked field", "[parser]")
{
    auto j = R"({"type":"Button","rect":[10,20,100,25],"locked":true})";
    auto ctrl = ParseControl(nlohmann::json::parse(j));
    REQUIRE(ctrl.locked == true);
}

TEST_CASE("ParseControl defaults locked to false when absent", "[parser]")
{
    auto j = R"({"type":"Button","rect":[10,20,100,25]})";
    auto ctrl = ParseControl(nlohmann::json::parse(j));
    REQUIRE(ctrl.locked == false);
}

TEST_CASE("SerializeControl omits locked when false", "[serializer]")
{
    auto ctrl = Control{};
    ctrl.type = ControlType::Button;
    ctrl.locked = false;
    auto j = SerializeControl(ctrl);
    REQUIRE_FALSE(j.contains("locked"));
}

TEST_CASE("SerializeControl includes locked when true", "[serializer]")
{
    auto ctrl = Control{};
    ctrl.type = ControlType::Button;
    ctrl.locked = true;
    auto j = SerializeControl(ctrl);
    REQUIRE(j.contains("locked"));
    REQUIRE(j["locked"].get<bool>() == true);
}

TEST_CASE("Locked field survives parse-serialize roundtrip", "[parser][serializer]")
{
    auto form = Form{};
    auto btn = Control{};
    btn.type = ControlType::Button;
    btn.text = L"Locked";
    btn.locked = true;
    form.controls.push_back(btn);

    auto lbl = Control{};
    lbl.type = ControlType::Label;
    lbl.text = L"Not locked";
    lbl.locked = false;
    form.controls.push_back(lbl);

    auto json = SerializeForm(form);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.controls[0].locked == true);
    REQUIRE(parsed.controls[1].locked == false);
}

// === Designer guides ===

TEST_CASE("ParseForm reads guides array", "[parser]")
{
    auto json = R"({
        "title": "Guided",
        "width": 400,
        "height": 300,
        "controls": [],
        "guides": [
            { "horizontal": true, "position": 100 },
            { "horizontal": false, "position": 200 }
        ]
    })";

    auto form = ParseForm(json);
    REQUIRE(form.guides.size() == 2);
    REQUIRE(form.guides[0].horizontal == true);
    REQUIRE(form.guides[0].position == 100);
    REQUIRE(form.guides[1].horizontal == false);
    REQUIRE(form.guides[1].position == 200);
}

TEST_CASE("ParseForm defaults to empty guides when absent", "[parser]")
{
    auto json = R"({"title":"NoGuides","width":400,"height":300,"controls":[]})";
    auto form = ParseForm(json);
    REQUIRE(form.guides.empty());
}

TEST_CASE("SerializeForm omits guides when empty", "[serializer]")
{
    Form form;
    auto json = SerializeForm(form);
    REQUIRE(json.find("guides") == std::string::npos);
}

TEST_CASE("SerializeForm includes guides when present", "[serializer]")
{
    Form form;
    form.guides.push_back({ true, 50 });
    form.guides.push_back({ false, 120 });
    auto json = SerializeForm(form);
    REQUIRE(json.find("guides") != std::string::npos);

    auto parsed = ParseForm(json);
    REQUIRE(parsed.guides.size() == 2);
    REQUIRE(parsed.guides[0].horizontal == true);
    REQUIRE(parsed.guides[0].position == 50);
    REQUIRE(parsed.guides[1].horizontal == false);
    REQUIRE(parsed.guides[1].position == 120);
}

TEST_CASE("Guides survive parse-serialize roundtrip", "[parser][serializer]")
{
    Form form;
    form.guides.push_back({ true, 75 });
    form.guides.push_back({ false, 300 });

    auto json = SerializeForm(form);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.guides.size() == 2);
    REQUIRE(parsed.guides[0].horizontal == true);
    REQUIRE(parsed.guides[0].position == 75);
    REQUIRE(parsed.guides[1].horizontal == false);
    REQUIRE(parsed.guides[1].position == 300);
}

// === Anchor round-trip ===

TEST_CASE("Anchor default (Top|Left) is not serialized", "[serializer]")
{
    Control c;
    c.type = ControlType::Button;
    c.anchor = Anchor::Top | Anchor::Left;

    Form form;
    form.controls.push_back(c);
    auto json = SerializeForm(form);

    REQUIRE(json.find("anchor") == std::string::npos);
}

TEST_CASE("Anchor non-default round-trips through JSON", "[parser][serializer]")
{
    Control c;
    c.type = ControlType::TextBox;
    c.anchor = Anchor::Top | Anchor::Left | Anchor::Right;

    Form form;
    form.controls.push_back(c);
    auto json = SerializeForm(form);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.controls[0].anchor == (Anchor::Top | Anchor::Left | Anchor::Right));
}

TEST_CASE("Anchor All flags round-trip through JSON", "[parser][serializer]")
{
    Control c;
    c.type = ControlType::ListView;
    c.anchor = Anchor::Top | Anchor::Bottom | Anchor::Left | Anchor::Right;

    Form form;
    form.controls.push_back(c);
    auto json = SerializeForm(form);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.controls[0].anchor == (Anchor::Top | Anchor::Bottom | Anchor::Left | Anchor::Right));
}

TEST_CASE("Anchor missing from JSON defaults to Top|Left", "[parser]")
{
    auto json = R"({"title":"Test","width":400,"height":300,"controls":[{"type":"Button","rect":[0,0,80,25]}]})";
    auto form = ParseForm(json);

    REQUIRE(form.controls[0].anchor == (Anchor::Top | Anchor::Left));
}

// === Font parse and serialize tests ===

TEST_CASE("ParseForm parses control font", "[parser][font]")
{
    auto json = R"({
        "title":"Test","width":400,"height":300,
        "controls":[{
            "type":"Button","rect":[10,10,80,25],
            "font":{"family":"Consolas","size":12,"bold":true,"italic":false}
        }]
    })";
    auto form = ParseForm(json);
    REQUIRE(form.controls[0].font.isSet());
    REQUIRE(form.controls[0].font.family == L"Consolas");
    REQUIRE(form.controls[0].font.size == 12);
    REQUIRE(form.controls[0].font.bold == true);
    REQUIRE(form.controls[0].font.italic == false);
}

TEST_CASE("ParseForm parses form-level font", "[parser][font]")
{
    auto json = R"({
        "title":"Test","width":400,"height":300,
        "font":{"family":"Arial","size":10,"bold":false,"italic":true},
        "controls":[]
    })";
    auto form = ParseForm(json);
    REQUIRE(form.font.isSet());
    REQUIRE(form.font.family == L"Arial");
    REQUIRE(form.font.size == 10);
    REQUIRE(form.font.bold == false);
    REQUIRE(form.font.italic == true);
}

TEST_CASE("ParseForm handles partial font (only some fields)", "[parser][font]")
{
    auto json = R"({
        "title":"Test","width":400,"height":300,
        "controls":[{
            "type":"Label","rect":[0,0,100,20],
            "font":{"size":14}
        }]
    })";
    auto form = ParseForm(json);
    REQUIRE(form.controls[0].font.isSet());
    REQUIRE(form.controls[0].font.family.empty());
    REQUIRE(form.controls[0].font.size == 14);
    REQUIRE(form.controls[0].font.bold == false);
    REQUIRE(form.controls[0].font.italic == false);
}

TEST_CASE("ParseForm handles missing font (defaults to unset)", "[parser][font]")
{
    auto json = R"({"title":"Test","width":400,"height":300,"controls":[{"type":"Button","rect":[0,0,80,25]}]})";
    auto form = ParseForm(json);
    REQUIRE_FALSE(form.controls[0].font.isSet());
    REQUIRE_FALSE(form.font.isSet());
}

TEST_CASE("SerializeForm omits font when not set", "[serializer][font]")
{
    Form form;
    form.controls.emplace_back();
    form.controls[0].type = ControlType::Button;
    form.controls[0].text = L"OK";

    auto json = SerializeForm(form);
    REQUIRE(json.find("\"font\"") == std::string::npos);
}

TEST_CASE("SerializeForm includes control font when set", "[serializer][font]")
{
    Form form;
    form.controls.emplace_back();
    form.controls[0].type = ControlType::Button;
    form.controls[0].font.family = L"Consolas";
    form.controls[0].font.size = 12;
    form.controls[0].font.bold = true;

    auto json = SerializeForm(form);
    REQUIRE(json.find("\"font\"") != std::string::npos);
    REQUIRE(json.find("Consolas") != std::string::npos);
}

TEST_CASE("SerializeForm includes form-level font when set", "[serializer][font]")
{
    Form form;
    form.font.family = L"Arial";
    form.font.size = 10;
    form.font.italic = true;

    auto json = SerializeForm(form);
    REQUIRE(json.find("\"font\"") != std::string::npos);
    REQUIRE(json.find("Arial") != std::string::npos);
}

TEST_CASE("Font round-trips through serialize/parse", "[parser][serializer][font]")
{
    Form original;
    original.font.family = L"Verdana";
    original.font.size = 11;
    original.font.bold = true;
    original.font.italic = false;

    original.controls.emplace_back();
    original.controls[0].type = ControlType::Label;
    original.controls[0].font.family = L"Courier New";
    original.controls[0].font.size = 14;
    original.controls[0].font.italic = true;

    auto json = SerializeForm(original);
    auto parsed = ParseForm(json);

    REQUIRE(parsed.font.family == L"Verdana");
    REQUIRE(parsed.font.size == 11);
    REQUIRE(parsed.font.bold == true);
    REQUIRE(parsed.font.italic == false);

    REQUIRE(parsed.controls[0].font.family == L"Courier New");
    REQUIRE(parsed.controls[0].font.size == 14);
    REQUIRE(parsed.controls[0].font.italic == true);
    REQUIRE(parsed.controls[0].font.bold == false);
}
