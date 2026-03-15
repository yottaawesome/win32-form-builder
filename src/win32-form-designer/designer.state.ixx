export module designer:state;
import std;
import formbuilder;
import :win32;

export namespace Designer
{

// Menu command IDs.
constexpr Win32::UINT IDM_FILE_NEW       = 40001;
constexpr Win32::UINT IDM_FILE_OPEN      = 40002;
constexpr Win32::UINT IDM_FILE_SAVE      = 40003;
constexpr Win32::UINT IDM_FILE_SAVE_AS   = 40004;
constexpr Win32::UINT IDM_FILE_EXIT      = 40005;
constexpr Win32::UINT IDM_CANCEL_PLACE   = 40006;

// Toolbox and layout constants.
constexpr int TOOLBOX_WIDTH  = 140;
constexpr int PROPERTY_WIDTH = 220;
constexpr Win32::UINT IDC_TOOLBOX   = 50001;

// Control property edit IDs.
constexpr Win32::UINT IDC_PROP_TYPE    = 51001;
constexpr Win32::UINT IDC_PROP_TEXT    = 51002;
constexpr Win32::UINT IDC_PROP_ID      = 51003;
constexpr Win32::UINT IDC_PROP_X       = 51004;
constexpr Win32::UINT IDC_PROP_Y       = 51005;
constexpr Win32::UINT IDC_PROP_W       = 51006;
constexpr Win32::UINT IDC_PROP_H       = 51007;
constexpr Win32::UINT IDC_PROP_ONCLICK = 51008;

// Form property edit IDs.
constexpr Win32::UINT IDC_PROP_FORM_TITLE      = 52001;
constexpr Win32::UINT IDC_PROP_FORM_WIDTH      = 52002;
constexpr Win32::UINT IDC_PROP_FORM_HEIGHT     = 52003;
constexpr Win32::UINT IDC_PROP_FORM_BGCOLOR    = 52004;
constexpr Win32::UINT IDC_PROP_FORM_BGCOLOR_BTN = 52005;

// Label IDs are offset from the corresponding edit IDs.
constexpr Win32::UINT IDL_OFFSET = 10000;

struct ControlEntry
{
    FormDesigner::Control* control;
    Win32::HWND hwnd;
};

enum class DragMode { None, Move, Resize };

constexpr int HANDLE_SIZE = 6;
constexpr int HANDLE_HALF = HANDLE_SIZE / 2;
constexpr int MIN_CONTROL_SIZE = 10;

// Alignment guide shown during drag operations.
struct AlignGuide
{
    bool horizontal;  // true = horizontal line (shared Y), false = vertical (shared X)
    int position;     // pixel coordinate on the guide's axis
};

constexpr int SNAP_THRESHOLD = 5;

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
    Win32::HINSTANCE hInstance = nullptr;
    Win32::HWND surfaceHwnd = nullptr;
    Win32::HWND canvasHwnd = nullptr;
    Win32::HWND toolboxHwnd = nullptr;
    Win32::HWND propertyHwnd = nullptr;
    std::vector<ControlEntry> entries;
    int selectedIndex = -1;

    DragMode dragMode = DragMode::None;
    int activeHandle = -1;
    Win32::POINT dragStart = {};
    Win32::POINT controlStart = {};
    Win32::SIZE controlStartSize = {};

    bool placementMode = false;
    FormDesigner::ControlType placementType = FormDesigner::ControlType::Button;

    bool updatingProperties = false;

    std::vector<AlignGuide> guides;

    std::filesystem::path currentFile;
    bool dirty = false;
};

constexpr Win32::UINT SUBCLASS_ID = 1;

}
