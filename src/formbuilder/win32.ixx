module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <Richedit.h>
#include <Commdlg.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <uxtheme.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

export module formbuilder:win32;

export namespace Win32
{
	// ===================================================================
	// Types
	// ===================================================================

	using
		::HWND,
		::HINSTANCE,
		::HMENU,
		::HBRUSH,
		::HPEN,
		::HCURSOR,
		::HFONT,
		::HDC,
		::HGDIOBJ,
		::DWORD,
		::DWORD_PTR,
		::WORD,
		::UINT,
		::INT_PTR,
		::UINT_PTR,
		::LONG_PTR,
		::BOOL,
		::WPARAM,
		::LPARAM,
		::LRESULT,
		::LPWSTR,
		::LPCWSTR,
		::MSG,
		::RECT,
		::POINT,
		::SIZE,
		::WNDCLASSEXW,
		::INITCOMMONCONTROLSEX,
		::PAINTSTRUCT,
		::COLORREF,
		::SCROLLINFO,
		::OPENFILENAMEW,
		::CHOOSECOLORW,
		::LOGFONTW,
		::CHOOSEFONTW,
		::HACCEL,
		::ACCEL,
		::NMHDR,
		::TTTOOLINFOW,
		::TBBUTTON,
		::NMTBGETINFOTIPW,
		::WINDOWPLACEMENT
		;

	// ===================================================================
	// Functions
	// ===================================================================

	using
		// Window lifecycle.
		::CreateWindowExW,
		::DestroyWindow,
		::DefWindowProcW,
		::RegisterClassExW,
		::UnregisterClassW,
		::ShowWindow,
		::UpdateWindow,
		::IsWindow,

		// Message loop.
		::GetMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::PostQuitMessage,
		::SendMessageW,
		::MessageBoxW,

		// Window properties.
		::GetWindowLongPtrW,
		::SetWindowLongPtrW,
		::SetWindowTextW,
		::GetWindowTextW,
		::GetWindowTextLengthW,
		::SetWindowPos,
		::MoveWindow,
		::EnableWindow,
		::SetFocus,
		::SetForegroundWindow,
		::GetWindowRect,
		::GetClientRect,
		::InvalidateRect,
		::GetWindowPlacement,
		::SetWindowPlacement,

		// Dialog item helpers.
		::GetDlgItem,
		::GetDlgCtrlID,
		::SetDlgItemTextW,
		::GetDlgItemTextW,
		::SetDlgItemInt,
		::GetDlgItemInt,

		// Graphics / DC.
		::GetDC,
		::ReleaseDC,
		::BeginPaint,
		::EndPaint,
		::CreatePen,
		::CreateSolidBrush,
		::CreateHatchBrush,
		::CreateFontW,
		::SelectObject,
		::DeleteObject,
		::FillRect,
		::Rectangle,
		::Ellipse,
		::MoveToEx,
		::LineTo,
		::SetPixel,
		::SetBkMode,
		::SetBkColor,
		::SetTextColor,
		::TextOutW,
		::DrawTextW,
		::CreateCompatibleDC,
		::CreateCompatibleBitmap,
		::BitBlt,
		::DeleteDC,
		::GetStockObject,
		::GetDeviceCaps,
		::MulDiv,

		// Input.
		::LoadCursorW,
		::SetCursor,
		::SetCapture,
		::ReleaseCapture,
		::GetKeyState,

		// Scrolling.
		::SetScrollInfo,
		::GetScrollInfo,
		::ScrollWindowEx,

		// Menus.
		::CreateMenu,
		::CreatePopupMenu,
		::AppendMenuW,
		::InsertMenuW,
		::DeleteMenu,
		::DestroyMenu,
		::GetMenu,
		::GetSubMenu,
		::GetMenuItemCount,
		::GetMenuItemID,
		::CheckMenuItem,
		::EnableMenuItem,
		::TrackPopupMenu,

		// Accelerators.
		::CreateAcceleratorTableW,
		::DestroyAcceleratorTable,
		::TranslateAcceleratorW,

		// Common dialogs.
		::GetSaveFileNameW,
		::GetOpenFileNameW,
		::ChooseColorW,
		::ChooseFontW,

		// Subclassing.
		::SetWindowSubclass,
		::DefSubclassProc,

		// Coordinate mapping.
		::ClientToScreen,

		// Miscellaneous.
		::AdjustWindowRectEx,
		::AdjustWindowRectExForDpi,
		::InitCommonControlsEx,
		::LoadLibraryW,
		::GetLastError,
		::GetSysColor,
		::EnumChildWindows,
		::GetModuleFileNameW,

		// DPI awareness.
		::SetProcessDpiAwarenessContext,
		::GetDpiForWindow,
		::GetDpiForSystem,
		::GetSystemMetricsForDpi
		;

	// ===================================================================
	// Macro wrappers
	// ===================================================================

	auto GetHighWord(WPARAM wParam) noexcept -> WORD { return HIWORD(wParam); }
	auto GetLowWord(WPARAM wParam) noexcept -> WORD { return LOWORD(wParam); }
	auto GetXParam(LPARAM lp) noexcept -> int { return GET_X_LPARAM(lp); }
	auto GetYParam(LPARAM lp) noexcept -> int { return GET_Y_LPARAM(lp); }
	auto MakeRgb(int r, int g, int b) noexcept -> COLORREF { return RGB(r, g, b); }
	auto GetWheelDelta(WPARAM wParam) noexcept -> short { return GET_WHEEL_DELTA_WPARAM(wParam); }

	// DPI context constants (cannot export macros directly).
	inline const auto DpiContextPerMonitorAwareV2 = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
	constexpr int DefaultDpi = USER_DEFAULT_SCREEN_DPI;  // 96

	inline auto ScaleDpi(int value, int dpi) noexcept -> int
	{
		return ::MulDiv(value, dpi, USER_DEFAULT_SCREEN_DPI);
	}

	// ===================================================================
	// Window & control styles
	// ===================================================================

	namespace Styles
	{
		// Window styles.
		constexpr auto Overlapped       = WS_OVERLAPPED;
		constexpr auto OverlappedWindow = WS_OVERLAPPEDWINDOW;
		constexpr auto Child            = WS_CHILD;
		constexpr auto Visible          = WS_VISIBLE;
		constexpr auto Border           = WS_BORDER;
		constexpr auto Caption          = WS_CAPTION;
		constexpr auto SysMenu          = WS_SYSMENU;
		constexpr auto ThickFrame       = WS_THICKFRAME;
		constexpr auto MinimizeBox      = WS_MINIMIZEBOX;
		constexpr auto MaximizeBox      = WS_MAXIMIZEBOX;
		constexpr auto TabStop          = WS_TABSTOP;
		constexpr auto VScroll          = WS_VSCROLL;

		// Button styles.
		constexpr auto PushButton       = BS_DEFPUSHBUTTON;
		constexpr auto ButtonPush       = BS_PUSHBUTTON;
		constexpr auto AutoCheckBox     = BS_AUTOCHECKBOX;
		constexpr auto AutoRadioButton  = BS_AUTORADIOBUTTON;
		constexpr auto GroupBox         = BS_GROUPBOX;
		constexpr auto ButtonLeft       = BS_LEFT;
		constexpr auto ButtonCenter     = BS_CENTER;
		constexpr auto ButtonRight      = BS_RIGHT;

		// Edit styles.
		constexpr auto EditAutoHScroll  = ES_AUTOHSCROLL;
		constexpr auto EditAutoVScroll  = ES_AUTOVSCROLL;
		constexpr auto EditCenter       = ES_CENTER;
		constexpr auto EditRight        = ES_RIGHT;
		constexpr auto EditMultiLine    = ES_MULTILINE;
		constexpr auto EditWantReturn   = ES_WANTRETURN;
		constexpr auto EditReadOnly     = ES_READONLY;

		// Static styles.
		constexpr auto StaticLeft        = SS_LEFT;
		constexpr auto StaticCenter      = SS_CENTER;
		constexpr auto StaticRight       = SS_RIGHT;
		constexpr auto StaticEtchedFrame = SS_ETCHEDFRAME;
		constexpr auto StaticEtchedHorz  = SS_ETCHEDHORZ;

		// ListBox / ComboBox styles.
		constexpr auto ListBoxStandard          = LBS_STANDARD;
		constexpr auto ListBoxNotify            = LBS_NOTIFY;
		constexpr auto ListBoxNoIntegralHeight  = LBS_NOINTEGRALHEIGHT;
		constexpr auto ComboBoxDropDownList      = CBS_DROPDOWNLIST;

		// ListView / TreeView styles.
		constexpr auto ListViewReport          = LVS_REPORT;
		constexpr auto ListViewShowSelAlways   = LVS_SHOWSELALWAYS;
		constexpr auto TreeViewHasButtons      = TVS_HASBUTTONS;
		constexpr auto TreeViewHasLines        = TVS_HASLINES;
		constexpr auto TreeViewLinesAtRoot     = TVS_LINESATROOT;
	}

	namespace ExStyles
	{
		constexpr auto ClientEdge = WS_EX_CLIENTEDGE;
	}

	// Class styles.
	constexpr auto Cs_HRedraw = CS_HREDRAW;
	constexpr auto Cs_VRedraw = CS_VREDRAW;

	// ===================================================================
	// Messages
	// ===================================================================

	namespace Messages
	{
		constexpr auto Command      = WM_COMMAND;
		constexpr auto Notify       = WM_NOTIFY;
		constexpr auto Destroy      = WM_DESTROY;
		constexpr auto SetFont      = WM_SETFONT;
		constexpr auto EraseBkgnd   = WM_ERASEBKGND;
		constexpr auto Size         = WM_SIZE;
		constexpr auto Close        = WM_CLOSE;
		constexpr auto Paint        = WM_PAINT;
		constexpr auto NcHitTest    = WM_NCHITTEST;
		constexpr auto NcDestroy    = WM_NCDESTROY;
		constexpr auto LButtonDown  = WM_LBUTTONDOWN;
		constexpr auto LButtonUp    = WM_LBUTTONUP;
		constexpr auto RButtonUp    = WM_RBUTTONUP;
		constexpr auto MouseMove    = WM_MOUSEMOVE;
		constexpr auto MouseWheel   = WM_MOUSEWHEEL;
		constexpr auto SetCursorMsg = WM_SETCURSOR;
		constexpr auto KeyDown      = WM_KEYDOWN;
		constexpr auto VScroll      = WM_VSCROLL;
		constexpr auto CtlColorEdit = WM_CTLCOLOREDIT;
		constexpr auto InitMenuPopup = WM_INITMENUPOPUP;
		constexpr auto DpiChanged    = WM_DPICHANGED;
	}

	// ===================================================================
	// Notifications
	// ===================================================================

	// WM_COMMAND notifications.
	namespace Notifications
	{
		constexpr auto ButtonClicked       = BN_CLICKED;
		constexpr auto ButtonDoubleClicked = BN_DBLCLK;
		constexpr auto EditChange          = EN_CHANGE;
		constexpr auto EditSetFocus        = EN_SETFOCUS;
		constexpr auto EditKillFocus       = EN_KILLFOCUS;
		constexpr auto ListBoxSelChange    = LBN_SELCHANGE;
		constexpr auto ListBoxDoubleClick  = LBN_DBLCLK;
		constexpr auto ListBoxSetFocus     = LBN_SETFOCUS;
		constexpr auto ListBoxKillFocus    = LBN_KILLFOCUS;
		constexpr auto ComboBoxSelChange   = CBN_SELCHANGE;
		constexpr auto ComboBoxSetFocus    = CBN_SETFOCUS;
		constexpr auto ComboBoxKillFocus   = CBN_KILLFOCUS;
		constexpr auto ComboBoxSelEndOk    = CBN_SELENDOK;
	}

	// WM_NOTIFY codes.
	namespace NotifyCodes
	{
		constexpr auto Click               = NM_CLICK;
		constexpr auto DoubleClick         = NM_DBLCLK;
		constexpr auto DateTimeChange      = DTN_DATETIMECHANGE;
		constexpr auto MonthCalSelChange   = MCN_SELCHANGE;
		constexpr auto IPAddressFieldChange = IPN_FIELDCHANGED;
		constexpr auto TreeViewSelChanged  = TVN_SELCHANGEDW;
		constexpr auto ListViewItemChanged = LVN_ITEMCHANGED;
		constexpr auto TabSelChange        = TCN_SELCHANGE;
		constexpr auto ToolbarGetInfoTip   = TBN_GETINFOTIPW;
	}

	// ===================================================================
	// Control messages
	// ===================================================================

	namespace Button
	{
		constexpr auto GetCheck = BM_GETCHECK;
		constexpr auto SetCheck = BM_SETCHECK;
		constexpr auto Checked  = BST_CHECKED;
		constexpr auto Unchecked = BST_UNCHECKED;
	}

	namespace ComboBox
	{
		constexpr auto AddString    = CB_ADDSTRING;
		constexpr auto SetCurSel    = CB_SETCURSEL;
		constexpr auto GetCurSel    = CB_GETCURSEL;
		constexpr auto ResetContent = CB_RESETCONTENT;
	}

	namespace ListBox
	{
		constexpr auto AddString    = LB_ADDSTRING;
		constexpr auto SetCurSel    = LB_SETCURSEL;
		constexpr auto GetCurSel    = LB_GETCURSEL;
		constexpr auto ResetContent = LB_RESETCONTENT;
	}

	// ===================================================================
	// Control class names
	// ===================================================================

	namespace Controls
	{
		constexpr auto Button        = WC_BUTTON;
		constexpr auto Static        = WC_STATIC;
		constexpr auto Edit          = WC_EDIT;
		constexpr auto ListBox       = WC_LISTBOX;
		constexpr auto ComboBox      = WC_COMBOBOX;
		constexpr auto Progress      = PROGRESS_CLASS;
		constexpr auto TrackBar      = TRACKBAR_CLASS;
		constexpr auto DateTimePick  = DATETIMEPICK_CLASS;
		constexpr auto Tab           = WC_TABCONTROL;
		constexpr auto ListView      = WC_LISTVIEW;
		constexpr auto TreeView      = WC_TREEVIEW;
		constexpr auto UpDown        = UPDOWN_CLASS;
		constexpr auto RichEdit      = MSFTEDIT_CLASS;
		constexpr auto MonthCalendar = MONTHCAL_CLASS;
		constexpr auto Link          = WC_LINK;
		constexpr auto IPAddress     = WC_IPADDRESS;
		constexpr auto HotKey        = HOTKEY_CLASS;
		constexpr auto Animation     = ANIMATE_CLASS;
		constexpr auto Tooltips      = TOOLTIPS_CLASS;
		constexpr auto Toolbar       = TOOLBARCLASSNAMEW;
		constexpr auto StatusBar     = STATUSCLASSNAMEW;
	}

	// ===================================================================
	// Tooltip constants
	// ===================================================================

	namespace TooltipStyles
	{
		constexpr auto AlwaysTip = TTS_ALWAYSTIP;
		constexpr auto NoPrefix  = TTS_NOPREFIX;
	}

	namespace TooltipFlags
	{
		constexpr auto Subclass  = TTF_SUBCLASS;
		constexpr auto IdIsHwnd  = TTF_IDISHWND;
	}

	namespace TooltipMessages
	{
		constexpr auto AddTool         = TTM_ADDTOOLW;
		constexpr auto DelTool         = TTM_DELTOOLW;
		constexpr auto UpdateTipText   = TTM_UPDATETIPTEXTW;
		constexpr auto SetMaxTipWidth  = TTM_SETMAXTIPWIDTH;
	}

	// ===================================================================
	// Virtual keys
	// ===================================================================

	namespace Keys
	{
		constexpr auto Delete  = VK_DELETE;
		constexpr auto Escape  = VK_ESCAPE;
		constexpr auto Control = VK_CONTROL;
		constexpr auto Shift   = VK_SHIFT;
		constexpr auto Left    = VK_LEFT;
		constexpr auto Right   = VK_RIGHT;
		constexpr auto Up      = VK_UP;
		constexpr auto Down    = VK_DOWN;
		constexpr auto F5      = VK_F5;
	}

	// ===================================================================
	// Hit test values
	// ===================================================================

	namespace HitTestValues
	{
		constexpr auto Transparent = HTTRANSPARENT;
		constexpr auto Client      = HTCLIENT;
	}

	// ===================================================================
	// Cursor resources
	// ===================================================================

	namespace Cursors
	{
		const auto Arrow    = IDC_ARROW;
		const auto Cross    = IDC_CROSS;
		const auto SizeNWSE = IDC_SIZENWSE;
		const auto SizeNESW = IDC_SIZENESW;
		const auto SizeNS   = IDC_SIZENS;
		const auto SizeWE   = IDC_SIZEWE;
	}

	// ===================================================================
	// Menu flags
	// ===================================================================

	namespace Menu
	{
		constexpr auto String     = MF_STRING;
		constexpr auto Separator  = MF_SEPARATOR;
		constexpr auto Grayed     = MF_GRAYED;
		constexpr auto Enabled    = MF_ENABLED;
		constexpr auto Checked    = MF_CHECKED;
		constexpr auto Unchecked  = MF_UNCHECKED;
		constexpr auto Popup      = MF_POPUP;
		constexpr auto ByPosition = MF_BYPOSITION;
		constexpr auto ByCommand  = MF_BYCOMMAND;
	}

	namespace TrackPopup
	{
		constexpr auto LeftAlign   = TPM_LEFTALIGN;
		constexpr auto TopAlign    = TPM_TOPALIGN;
		constexpr auto ReturnCmd   = TPM_RETURNCMD;
		constexpr auto RightButton = TPM_RIGHTBUTTON;
	}

	// ===================================================================
	// Scroll constants
	// ===================================================================

	namespace ScrollBar
	{
		constexpr auto Vert          = SB_VERT;
		constexpr auto LineUp        = SB_LINEUP;
		constexpr auto LineDown      = SB_LINEDOWN;
		constexpr auto PageUp        = SB_PAGEUP;
		constexpr auto PageDown      = SB_PAGEDOWN;
		constexpr auto ThumbTrack    = SB_THUMBTRACK;
		constexpr auto ThumbPosition = SB_THUMBPOSITION;
		constexpr auto Top           = SB_TOP;
		constexpr auto Bottom        = SB_BOTTOM;
	}

	namespace ScrollInfo
	{
		constexpr auto Range = SIF_RANGE;
		constexpr auto Page  = SIF_PAGE;
		constexpr auto Pos   = SIF_POS;
		constexpr auto All   = SIF_ALL;
	}

	namespace ScrollWindow
	{
		constexpr auto ScrollChildren = SW_SCROLLCHILDREN;
		constexpr auto Invalidate     = SW_INVALIDATE;
		constexpr auto Erase          = SW_ERASE;
	}

	// ===================================================================
	// Drawing constants
	// ===================================================================

	namespace PenStyles
	{
		constexpr auto Solid = PS_SOLID;
		constexpr auto Dot   = PS_DOT;
		constexpr auto Dash  = PS_DASH;
	}

	namespace DrawTextFlags
	{
		constexpr auto Center     = DT_CENTER;
		constexpr auto VCenter    = DT_VCENTER;
		constexpr auto SingleLine = DT_SINGLELINE;
	}

	constexpr auto Bk_Transparent = TRANSPARENT;
	constexpr auto NullBrush      = NULL_BRUSH;
	constexpr auto NullPen        = NULL_PEN;
	constexpr auto SrcCopy        = SRCCOPY;
	constexpr auto HatchBDiagonal = HS_BDIAGONAL;

	// ===================================================================
	// Dialog constants
	// ===================================================================

	namespace FileDialog
	{
		constexpr auto OverwritePrompt = OFN_OVERWRITEPROMPT;
		constexpr auto PathMustExist   = OFN_PATHMUSTEXIST;
		constexpr auto FileMustExist   = OFN_FILEMUSTEXIST;
	}

	namespace ColorDialog
	{
		constexpr auto FullOpen = CC_FULLOPEN;
		constexpr auto RgbInit  = CC_RGBINIT;
	}

	namespace FontDialog
	{
		constexpr auto ScreenFonts   = CF_SCREENFONTS;
		constexpr auto InitToLogFont = CF_INITTOLOGFONTSTRUCT;
		constexpr auto NoSimulations = CF_NOSIMULATIONS;
	}

	// ===================================================================
	// Toolbar & status bar
	// ===================================================================

	namespace Toolbar
	{
		constexpr auto ButtonStructSize = TB_BUTTONSTRUCTSIZE;
		constexpr auto AddButtons       = TB_ADDBUTTONSW;
		constexpr auto AutoSize         = TB_AUTOSIZE;
		constexpr auto SetBitmapSize    = TB_SETBITMAPSIZE;
		constexpr auto EnableButton     = TB_ENABLEBUTTON;
	}

	namespace ToolbarStyle
	{
		constexpr auto Flat     = TBSTYLE_FLAT;
		constexpr auto List     = TBSTYLE_LIST;
		constexpr auto Tooltips = TBSTYLE_TOOLTIPS;
	}

	namespace CommonControlStyle
	{
		constexpr auto Top = CCS_TOP;
	}

	namespace StatusBarStyle
	{
		constexpr DWORD SizeGrip = SBARS_SIZEGRIP;
	}

	namespace ButtonStyle
	{
		constexpr BYTE Button   = BTNS_BUTTON;
		constexpr BYTE Sep      = BTNS_SEP;
		constexpr BYTE ShowText = BTNS_SHOWTEXT;
		constexpr BYTE Enabled  = TBSTATE_ENABLED;
	}

	namespace StatusBar
	{
		constexpr auto SetParts = SB_SETPARTS;
		constexpr auto SetTextW = SB_SETTEXTW;
	}

	// ===================================================================
	// Accelerator flags
	// ===================================================================

	namespace Accel
	{
		constexpr auto Control = FCONTROL;
		constexpr auto Shift   = FSHIFT;
		constexpr auto VirtKey = FVIRTKEY;
	}

	// ===================================================================
	// Font constants
	// ===================================================================

	constexpr auto FwNormal          = FW_NORMAL;
	constexpr auto FwBold            = FW_BOLD;
	constexpr auto DefaultCharset    = DEFAULT_CHARSET;
	constexpr auto OutDefaultPrecis  = OUT_DEFAULT_PRECIS;
	constexpr auto ClipDefaultPrecis = CLIP_DEFAULT_PRECIS;
	constexpr auto ClearTypeQuality  = CLEARTYPE_QUALITY;
	constexpr auto DefaultPitch      = DEFAULT_PITCH;
	constexpr auto FfDontCare        = FF_DONTCARE;

	namespace FontWeight
	{
		constexpr auto Normal = FW_NORMAL;
		constexpr auto Bold   = FW_BOLD;
	}

	namespace FontCharset
	{
		constexpr auto Default = DEFAULT_CHARSET;
	}

	namespace FontPrecision
	{
		constexpr auto OutDefault     = OUT_DEFAULT_PRECIS;
		constexpr auto ClipDefault    = CLIP_DEFAULT_PRECIS;
		constexpr auto QualityDefault = CLEARTYPE_QUALITY;
	}

	namespace FontMetrics
	{
		constexpr auto LogPixelsY = LOGPIXELSY;
	}

	// Creates an HFONT from font properties.
	inline auto CreateFontFromInfo(const wchar_t* family, int size, bool bold, bool italic, HWND hwnd = nullptr) -> HFONT
	{
		auto hdc = ::GetDC(hwnd);
		auto logPixelsY = ::GetDeviceCaps(hdc, LOGPIXELSY);
		::ReleaseDC(hwnd, hdc);
		int height = -::MulDiv(size, logPixelsY, 72);
		return ::CreateFontW(
			height, 0, 0, 0,
			bold ? FW_BOLD : FW_NORMAL,
			italic ? TRUE : FALSE,
			FALSE, FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			family
		);
	}

	// ===================================================================
	// System constants
	// ===================================================================

	constexpr auto Gwlp_UserData     = GWLP_USERDATA;
	constexpr auto GclpHbrBackground = GCLP_HBRBACKGROUND;
	constexpr auto Cw_UseDefault     = CW_USEDEFAULT;
	constexpr auto DefaultGuiFont    = DEFAULT_GUI_FONT;
	constexpr auto MaxPath           = MAX_PATH;
	constexpr auto ColorWindow       = COLOR_WINDOW;
	constexpr auto ColorBtnFace      = COLOR_BTNFACE;
	constexpr auto ColorAppWorkspace = COLOR_APPWORKSPACE;

	// Show window commands.
	constexpr auto Sw_ShowDefault  = SW_SHOWDEFAULT;
	constexpr auto Sw_ShowNormal   = SW_SHOWNORMAL;
	constexpr auto Sw_ShowMaximized = SW_SHOWMAXIMIZED;
	constexpr auto Sw_Show         = SW_SHOW;
	constexpr auto Sw_Hide         = SW_HIDE;

	// SetWindowPos constants.
	const auto HwndBottom = HWND_BOTTOM;
	const auto HwndTop    = HWND_TOP;

	namespace Swp
	{
		constexpr auto NoMove     = SWP_NOMOVE;
		constexpr auto NoSize     = SWP_NOSIZE;
		constexpr auto NoActivate = SWP_NOACTIVATE;
		constexpr auto NoZOrder   = SWP_NOZORDER;
	}

	// Common controls.
	constexpr auto Icc_StandardClasses = ICC_STANDARD_CLASSES;

	// MessageBox flags.
	constexpr auto Mb_Ok              = MB_OK;
	constexpr auto Mb_IconInformation = MB_ICONINFORMATION;
	constexpr auto Mb_IconError       = MB_ICONERROR;
	constexpr auto Mb_IconWarning     = MB_ICONWARNING;
	constexpr auto Mb_YesNoCancel     = MB_YESNOCANCEL;
	constexpr auto Mb_IconQuestion    = MB_ICONQUESTION;
	constexpr auto Id_Cancel          = IDCANCEL;
	constexpr auto Id_Yes             = IDYES;
	constexpr auto Id_No              = IDNO;

	// ===================================================================
	// Dark mode support
	// ===================================================================

	auto SetDarkTitleBar(HWND hwnd, bool dark) -> bool
	{
		BOOL value = dark ? TRUE : FALSE;
		return SUCCEEDED(DwmSetWindowAttribute(
			hwnd, 20, &value, sizeof(value)));
	}

	auto SetDarkScrollBars(HWND hwnd, bool dark) -> void
	{
		SetWindowTheme(hwnd, dark ? L"DarkMode_Explorer" : nullptr, nullptr);
	}

	auto SetClassBackground(HWND hwnd, HBRUSH brush) -> void
	{
		SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(brush));
	}
}
