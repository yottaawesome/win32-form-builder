export module designer:state;
import std;
import formbuilder;

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
	constexpr Win32::UINT IDM_EDIT_GROUP    = 40024;
	constexpr Win32::UINT IDM_EDIT_UNGROUP  = 40025;
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
	constexpr Win32::UINT IDM_VIEW_DARKMODE = 40022;
	constexpr Win32::UINT IDM_VIEW_TABORDER = 40023;

	// File menu IDs (continued).
	constexpr Win32::UINT IDM_FILE_PREVIEW = 40018;
	constexpr Win32::UINT IDM_FILE_EXPORT_CPP = 40021;
	constexpr Win32::UINT IDM_FILE_EXPORT_RC = 40026;

	constexpr Win32::UINT IDM_FILE_RECENT_BASE = 40200;
	constexpr int MAX_RECENT_FILES = 10;

	// Format menu IDs — alignment, distribution, sizing.
	constexpr Win32::UINT IDM_FORMAT_ALIGN_LEFT      = 40301;
	constexpr Win32::UINT IDM_FORMAT_ALIGN_CENTER_H  = 40302;
	constexpr Win32::UINT IDM_FORMAT_ALIGN_RIGHT     = 40303;
	constexpr Win32::UINT IDM_FORMAT_ALIGN_TOP       = 40304;
	constexpr Win32::UINT IDM_FORMAT_ALIGN_MIDDLE_V  = 40305;
	constexpr Win32::UINT IDM_FORMAT_ALIGN_BOTTOM    = 40306;
	constexpr Win32::UINT IDM_FORMAT_DIST_HORIZ      = 40307;
	constexpr Win32::UINT IDM_FORMAT_DIST_VERT       = 40308;
	constexpr Win32::UINT IDM_FORMAT_SAME_WIDTH      = 40309;
	constexpr Win32::UINT IDM_FORMAT_SAME_HEIGHT     = 40310;
	constexpr Win32::UINT IDM_FORMAT_SAME_SIZE       = 40311;

	// Toolbox control ID.
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

	// Visible checkbox for control properties.
	constexpr Win32::UINT IDC_PROP_VISIBLE = 51026;

	// Anchor combo for control properties.
	constexpr Win32::UINT IDC_PROP_ANCHOR = 51018;

	// Font property controls.
	constexpr Win32::UINT IDC_PROP_FONT_LABEL = 51019;
	constexpr Win32::UINT IDC_PROP_FONT_BTN   = 51020;
	constexpr Win32::UINT IDC_PROP_FONT_CLEAR = 51021;
	constexpr Win32::UINT IDC_PROP_TOOLTIP    = 51022;

	// Items property controls (ComboBox/ListBox).
	constexpr Win32::UINT IDC_PROP_ITEMS_LABEL = 51023;
	constexpr Win32::UINT IDC_PROP_ITEMS_BTN   = 51024;
	constexpr Win32::UINT IDC_PROP_SELINDEX    = 51025;

	// Validation property controls.
	constexpr Win32::UINT IDC_PROP_VAL_REQUIRED = 51027;
	constexpr Win32::UINT IDC_PROP_VAL_MINLEN   = 51028;
	constexpr Win32::UINT IDC_PROP_VAL_MAXLEN   = 51029;
	constexpr Win32::UINT IDC_PROP_VAL_PATTERN  = 51030;
	constexpr Win32::UINT IDC_PROP_VAL_MIN      = 51031;
	constexpr Win32::UINT IDC_PROP_VAL_MAX      = 51032;

	// Image path property controls (Picture only).
	constexpr Win32::UINT IDC_PROP_IMAGEPATH     = 51033;
	constexpr Win32::UINT IDC_PROP_IMAGEPATH_BTN = 51034;

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

	// Form font property controls.
	constexpr Win32::UINT IDC_PROP_FORM_FONT_LABEL = 52011;
	constexpr Win32::UINT IDC_PROP_FORM_FONT_BTN   = 52012;
	constexpr Win32::UINT IDC_PROP_FORM_FONT_CLEAR = 52013;

	// Label IDs are offset from the corresponding edit IDs.
	constexpr Win32::UINT IDL_OFFSET = 10000;

	// Z-order panel control IDs.
	constexpr Win32::UINT IDC_ZORDER_LIST   = 53001;
	constexpr Win32::UINT IDC_ZORDER_UP     = 53002;
	constexpr Win32::UINT IDC_ZORDER_DOWN   = 53003;
	constexpr Win32::UINT IDC_ZORDER_TOP    = 53004;
	constexpr Win32::UINT IDC_ZORDER_BOTTOM = 53005;
	constexpr Win32::UINT IDC_ZORDER_DELETE = 53006;
	constexpr Win32::UINT IDC_ZORDER_EDIT   = 53007;  // inline rename edit

	struct ControlEntry
	{
		FormDesigner::Control* control;
		Win32::HWND hwnd;
	};

	enum class DragMode { None, Move, Resize, CreateGuide, ResizeForm };

	// Which edge(s) of the form boundary are being dragged.
	enum class FormEdge { None, Right, Bottom, BottomRight };

	// Base pixel values at 96 DPI — scaled at runtime via DpiInfo.
	constexpr int BASE_HANDLE_SIZE = 6;
	constexpr int BASE_HANDLE_HALF = BASE_HANDLE_SIZE / 2;
	constexpr int BASE_MIN_CONTROL_SIZE = 10;

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

	constexpr int BASE_SNAP_THRESHOLD = 5;
	constexpr int BASE_DEFAULT_GRID_SIZE = 10;
	constexpr int BASE_RULER_SIZE = 20;
	constexpr int BASE_TOOLBOX_WIDTH = 140;
	constexpr int BASE_PROPERTY_WIDTH = 240;

	// Designer color theme.
	struct Theme
	{
		const wchar_t* name;
		bool isDark;
		Win32::COLORREF canvasBackground;
		Win32::COLORREF panelBackground;
		Win32::COLORREF formBorder;
		Win32::COLORREF gridDot;
		Win32::COLORREF selectionHighlight;
		Win32::COLORREF lockedHighlight;
		Win32::COLORREF handleFill;
		Win32::COLORREF rulerBackground;
		Win32::COLORREF rulerText;
		Win32::COLORREF rulerBorder;
		Win32::COLORREF rulerTick;
		Win32::COLORREF alignGuide;
		Win32::COLORREF userGuide;
		Win32::COLORREF formBoundary;
	};

	inline auto LightTheme() -> Theme
	{
		return {
			.name = L"Light",
			.isDark = false,
			.canvasBackground = Win32::GetSysColor(Win32::ColorAppWorkspace),
			.panelBackground = Win32::GetSysColor(Win32::ColorBtnFace),
			.formBorder = Win32::MakeRgb(128, 128, 128),
			.gridDot = Win32::MakeRgb(192, 192, 192),
			.selectionHighlight = Win32::MakeRgb(0, 120, 215),
			.lockedHighlight = Win32::MakeRgb(128, 128, 128),
			.handleFill = Win32::MakeRgb(255, 255, 255),
			.rulerBackground = Win32::MakeRgb(240, 240, 240),
			.rulerText = Win32::MakeRgb(80, 80, 80),
			.rulerBorder = Win32::MakeRgb(200, 200, 200),
			.rulerTick = Win32::MakeRgb(160, 160, 160),
			.alignGuide = Win32::MakeRgb(255, 0, 128),
			.userGuide = Win32::MakeRgb(0, 120, 215),
			.formBoundary = Win32::MakeRgb(255, 0, 0),
		};
	}

	inline auto DarkTheme() -> Theme
	{
		return {
			.name = L"Dark",
			.isDark = true,
			.canvasBackground = Win32::MakeRgb(30, 30, 30),
			.panelBackground = Win32::MakeRgb(45, 45, 45),
			.formBorder = Win32::MakeRgb(100, 100, 100),
			.gridDot = Win32::MakeRgb(70, 70, 70),
			.selectionHighlight = Win32::MakeRgb(0, 140, 255),
			.lockedHighlight = Win32::MakeRgb(100, 100, 100),
			.handleFill = Win32::MakeRgb(200, 200, 200),
			.rulerBackground = Win32::MakeRgb(50, 50, 50),
			.rulerText = Win32::MakeRgb(180, 180, 180),
			.rulerBorder = Win32::MakeRgb(70, 70, 70),
			.rulerTick = Win32::MakeRgb(90, 90, 90),
			.alignGuide = Win32::MakeRgb(255, 80, 160),
			.userGuide = Win32::MakeRgb(0, 160, 255),
			.formBoundary = Win32::MakeRgb(255, 80, 80),
		};
	}

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
		{ FormDesigner::ControlType::MonthCalendar,  L"MonthCalendar" },
		{ FormDesigner::ControlType::Link,           L"Link" },
		{ FormDesigner::ControlType::IPAddress,      L"IPAddress" },
		{ FormDesigner::ControlType::HotKey,         L"HotKey" },
		{ FormDesigner::ControlType::Picture,        L"Picture" },
		{ FormDesigner::ControlType::Separator,      L"Separator" },
		{ FormDesigner::ControlType::Animation,      L"Animation" },
	};

	// DPI scaling information. Updated on startup and WM_DPICHANGED.
	struct DpiInfo
	{
		int dpi = Win32::DefaultDpi;

		auto Scale(int baseValue) const noexcept -> int
		{
			return Win32::ScaleDpi(baseValue, dpi);
		}

		auto HandleSize()      const noexcept -> int { return Scale(BASE_HANDLE_SIZE); }
		auto HandleHalf()      const noexcept -> int { return Scale(BASE_HANDLE_HALF); }
		auto MinControlSize()  const noexcept -> int { return Scale(BASE_MIN_CONTROL_SIZE); }
		auto SnapThreshold()   const noexcept -> int { return Scale(BASE_SNAP_THRESHOLD); }
		auto RulerSize()       const noexcept -> int { return Scale(BASE_RULER_SIZE); }
		auto ToolboxWidth()    const noexcept -> int { return Scale(BASE_TOOLBOX_WIDTH); }
		auto PropertyWidth()   const noexcept -> int { return Scale(BASE_PROPERTY_WIDTH); }
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
		FormEdge formEdge = FormEdge::None;
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

		int gridSize = BASE_DEFAULT_GRID_SIZE;
		bool showGrid = true;
		bool snapToGrid = true;
		bool showRulers = true;
		std::vector<UserGuide> userGuides;
		bool draggingGuideHorizontal = false;
		int draggingGuidePos = -1;
		Win32::POINT lastCursorPos = { -1, -1 }; // form coordinates for ruler indicator
		Theme theme = LightTheme();
		std::set<Win32::UINT> invalidFields;
		bool tabOrderMode = false;
		int tabOrderNext = 1;
		int nextGroupId = 1;
		std::vector<std::filesystem::path> recentFiles;
		std::vector<Win32::HFONT> controlFonts; // Fonts created for design-time controls.
		Win32::HWND propTooltipHwnd = nullptr;   // Tooltip window for property panel buttons.
		DpiInfo dpiInfo;
	};

	constexpr Win32::UINT SUBCLASS_ID = 1;

}
