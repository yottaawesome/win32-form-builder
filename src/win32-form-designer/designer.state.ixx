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
constexpr Win32::UINT IDM_EDIT_UNDO      = 40007;
constexpr Win32::UINT IDM_EDIT_REDO      = 40008;
constexpr Win32::UINT IDM_EDIT_CUT       = 40009;
constexpr Win32::UINT IDM_EDIT_COPY      = 40010;
constexpr Win32::UINT IDM_EDIT_PASTE     = 40011;
constexpr Win32::UINT IDM_EDIT_DUPLICATE = 40012;

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
constexpr Win32::UINT IDC_PROP_ONCHANGE = 51009;
constexpr Win32::UINT IDC_PROP_ONDBLCLICK = 51010;
constexpr Win32::UINT IDC_PROP_ONSELCHANGE = 51011;
constexpr Win32::UINT IDC_PROP_ONFOCUS = 51012;
constexpr Win32::UINT IDC_PROP_ONBLUR = 51013;
constexpr Win32::UINT IDC_PROP_ONCHECK = 51014;

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
    { FormDesigner::ControlType::Button,      L"Button" },
    { FormDesigner::ControlType::CheckBox,    L"CheckBox" },
    { FormDesigner::ControlType::RadioButton, L"RadioButton" },
    { FormDesigner::ControlType::Label,       L"Label" },
    { FormDesigner::ControlType::TextBox,     L"TextBox" },
    { FormDesigner::ControlType::GroupBox,    L"GroupBox" },
    { FormDesigner::ControlType::ListBox,     L"ListBox" },
    { FormDesigner::ControlType::ComboBox,    L"ComboBox" },
    { FormDesigner::ControlType::ProgressBar,    L"ProgressBar" },
    { FormDesigner::ControlType::TrackBar,       L"TrackBar" },
    { FormDesigner::ControlType::DateTimePicker, L"DateTimePicker" },
    { FormDesigner::ControlType::TabControl,     L"TabControl" },
    { FormDesigner::ControlType::ListView,       L"ListView" },
    { FormDesigner::ControlType::TreeView,       L"TreeView" },
    { FormDesigner::ControlType::UpDown,         L"UpDown" },
    { FormDesigner::ControlType::RichEdit,       L"RichEdit" },
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
    std::set<int> selection;

    DragMode dragMode = DragMode::None;
    int activeHandle = -1;
    int dragAnchor = -1;
    Win32::POINT dragStart = {};
    Win32::POINT controlStart = {};
    Win32::SIZE controlStartSize = {};
    std::map<int, Win32::POINT> dragOrigins;

    bool placementMode = false;
    FormDesigner::ControlType placementType = FormDesigner::ControlType::Button;

    bool updatingProperties = false;

    std::vector<AlignGuide> guides;

    std::vector<FormDesigner::Form> undoStack;
    std::vector<FormDesigner::Form> redoStack;

    std::vector<FormDesigner::Control> clipboard;

    std::filesystem::path currentFile;
    bool dirty = false;
};

constexpr Win32::UINT SUBCLASS_ID = 1;

}
