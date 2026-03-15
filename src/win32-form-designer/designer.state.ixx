module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module designer:state;
import std;
import formbuilder;

export namespace Designer
{

// Menu command IDs.
constexpr UINT IDM_FILE_NEW       = 40001;
constexpr UINT IDM_FILE_OPEN      = 40002;
constexpr UINT IDM_FILE_SAVE      = 40003;
constexpr UINT IDM_FILE_SAVE_AS   = 40004;
constexpr UINT IDM_FILE_EXIT      = 40005;
constexpr UINT IDM_CANCEL_PLACE   = 40006;

// Toolbox and layout constants.
constexpr int TOOLBOX_WIDTH  = 140;
constexpr int PROPERTY_WIDTH = 220;
constexpr UINT IDC_TOOLBOX   = 50001;

// Control property edit IDs.
constexpr UINT IDC_PROP_TYPE    = 51001;
constexpr UINT IDC_PROP_TEXT    = 51002;
constexpr UINT IDC_PROP_ID      = 51003;
constexpr UINT IDC_PROP_X       = 51004;
constexpr UINT IDC_PROP_Y       = 51005;
constexpr UINT IDC_PROP_W       = 51006;
constexpr UINT IDC_PROP_H       = 51007;
constexpr UINT IDC_PROP_ONCLICK = 51008;

// Form property edit IDs.
constexpr UINT IDC_PROP_FORM_TITLE      = 52001;
constexpr UINT IDC_PROP_FORM_WIDTH      = 52002;
constexpr UINT IDC_PROP_FORM_HEIGHT     = 52003;
constexpr UINT IDC_PROP_FORM_BGCOLOR    = 52004;
constexpr UINT IDC_PROP_FORM_BGCOLOR_BTN = 52005;

// Label IDs are offset from the corresponding edit IDs.
constexpr UINT IDL_OFFSET = 10000;

struct ControlEntry
{
    FormDesigner::Control* control;
    HWND hwnd;
};

enum class DragMode { None, Move, Resize };

constexpr int HANDLE_SIZE = 6;
constexpr int HANDLE_HALF = HANDLE_SIZE / 2;
constexpr int MIN_CONTROL_SIZE = 10;

// Control types available in the toolbox.
struct ToolboxItem
{
    FormDesigner::ControlType type;
    const wchar_t* name;
};

inline constexpr ToolboxItem TOOLBOX_ITEMS[] = {
    { FormDesigner::ControlType::Button,   L"Button" },
    { FormDesigner::ControlType::CheckBox, L"CheckBox" },
    { FormDesigner::ControlType::Label,    L"Label" },
    { FormDesigner::ControlType::TextBox,  L"TextBox" },
    { FormDesigner::ControlType::GroupBox, L"GroupBox" },
    { FormDesigner::ControlType::ListBox,  L"ListBox" },
    { FormDesigner::ControlType::ComboBox, L"ComboBox" },
};

struct DesignState
{
    FormDesigner::Form form;
    HINSTANCE hInstance = nullptr;
    HWND surfaceHwnd = nullptr;
    HWND canvasHwnd = nullptr;
    HWND toolboxHwnd = nullptr;
    HWND propertyHwnd = nullptr;
    std::vector<ControlEntry> entries;
    int selectedIndex = -1;

    DragMode dragMode = DragMode::None;
    int activeHandle = -1;
    POINT dragStart = {};
    POINT controlStart = {};
    SIZE controlStartSize = {};

    bool placementMode = false;
    FormDesigner::ControlType placementType = FormDesigner::ControlType::Button;

    bool updatingProperties = false;

    std::filesystem::path currentFile;
    bool dirty = false;
};

constexpr UINT SUBCLASS_ID = 1;

}
