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
constexpr Win32::UINT IDM_EDIT_DELETE    = 40013;
constexpr Win32::UINT IDM_EDIT_SELECTALL = 40014;
constexpr Win32::UINT IDM_VIEW_ZORDER   = 40015;

// Context menu IDs.
constexpr Win32::UINT IDM_CTX_TOFRONT   = 40101;
constexpr Win32::UINT IDM_CTX_TOBACK    = 40102;
constexpr Win32::UINT IDM_CTX_LOCK      = 40103;

// View menu IDs.
constexpr Win32::UINT IDM_VIEW_SHOWGRID = 40016;
constexpr Win32::UINT IDM_VIEW_SNAPTOGRID = 40017;
constexpr Win32::UINT IDM_VIEW_SHOWRULERS = 40019;
constexpr Win32::UINT IDM_VIEW_CLEARGUIDES = 40020;

// File menu IDs (continued).
constexpr Win32::UINT IDM_FILE_PREVIEW = 40018;
constexpr Win32::UINT IDM_FILE_EXPORT_CPP = 40021;

// Toolbox and layout constants.
constexpr int TOOLBOX_WIDTH  = 140;
constexpr int PROPERTY_WIDTH = 240;
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
constexpr Win32::UINT IDC_PROP_TABINDEX = 51015;
constexpr Win32::UINT IDC_PROP_TEXTALIGN = 51016;

// Locked checkbox for control properties.
constexpr Win32::UINT IDC_PROP_LOCKED = 51017;

// Form property edit IDs.
constexpr Win32::UINT IDC_PROP_FORM_TITLE      = 52001;
constexpr Win32::UINT IDC_PROP_FORM_WIDTH      = 52002;
constexpr Win32::UINT IDC_PROP_FORM_HEIGHT     = 52003;
constexpr Win32::UINT IDC_PROP_FORM_BGCOLOR    = 52004;
constexpr Win32::UINT IDC_PROP_FORM_BGCOLOR_BTN = 52005;
constexpr Win32::UINT IDC_PROP_FORM_CAPTION    = 52006;
constexpr Win32::UINT IDC_PROP_FORM_SYSMENU    = 52007;
constexpr Win32::UINT IDC_PROP_FORM_RESIZABLE  = 52008;
constexpr Win32::UINT IDC_PROP_FORM_MINIMIZE   = 52009;
constexpr Win32::UINT IDC_PROP_FORM_MAXIMIZE   = 52010;

// Label IDs are offset from the corresponding edit IDs.
constexpr Win32::UINT IDL_OFFSET = 10000;

// Z-order panel control IDs.
constexpr Win32::UINT IDC_ZORDER_LIST   = 53001;
constexpr Win32::UINT IDC_ZORDER_UP     = 53002;
constexpr Win32::UINT IDC_ZORDER_DOWN   = 53003;
constexpr Win32::UINT IDC_ZORDER_TOP    = 53004;
constexpr Win32::UINT IDC_ZORDER_BOTTOM = 53005;

struct ControlEntry
{
    FormDesigner::Control* control;
    Win32::HWND hwnd;
};

enum class DragMode { None, Move, Resize, CreateGuide };

constexpr int HANDLE_SIZE = 6;
constexpr int HANDLE_HALF = HANDLE_SIZE / 2;
constexpr int MIN_CONTROL_SIZE = 10;

// Alignment guide shown during drag operations.
struct AlignGuide
{
    bool horizontal;  // true = horizontal line (shared Y), false = vertical (shared X)
    int position;     // pixel coordinate on the guide's axis
};

// User-created guide line dragged from a ruler.
struct UserGuide
{
    bool horizontal;  // true = horizontal (dragged from top ruler), false = vertical (from left ruler)
    int position;     // pixel coordinate in form space
};

constexpr int SNAP_THRESHOLD = 5;
constexpr int DEFAULT_GRID_SIZE = 10;
constexpr int RULER_SIZE = 20;

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
    int propertyScrollY = 0;

    std::vector<AlignGuide> guides;

    std::vector<FormDesigner::Form> undoStack;
    std::vector<FormDesigner::Form> redoStack;

    std::vector<FormDesigner::Control> clipboard;

    std::filesystem::path currentFile;
    bool dirty = false;
    Win32::HWND zorderHwnd = nullptr;
    Win32::HWND previewHwnd = nullptr;
    Win32::HWND toolbarHwnd = nullptr;
    Win32::HWND statusbarHwnd = nullptr;

    int gridSize = DEFAULT_GRID_SIZE;
    bool showGrid = true;
    bool snapToGrid = true;
    bool showRulers = true;
    std::vector<UserGuide> userGuides;
    bool draggingGuideHorizontal = false;
    int draggingGuidePos = -1;
    Win32::POINT lastCursorPos = { -1, -1 }; // form coordinates for ruler indicator
};

constexpr Win32::UINT SUBCLASS_ID = 1;

}
