module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <Richedit.h>

export module formbuilder:win32;

export namespace Win32
{
	// === Types ===
	using
		::HWND,
		::HINSTANCE,
		::HMENU,
		::HBRUSH,
		::HCURSOR,
		::HFONT,
		::HDC,
		::HGDIOBJ,
		::DWORD,
		::WORD,
		::UINT,
		::INT_PTR,
		::LONG_PTR,
		::BOOL,
		::WPARAM,
		::LPARAM,
		::LRESULT,
		::LPWSTR,
		::MSG,
		::RECT,
		::WNDCLASSEXW,
		::INITCOMMONCONTROLSEX
		;

	// === Functions ===
	using
		::CreateWindowExW,
		::DestroyWindow,
		::DefWindowProcW,
		::RegisterClassExW,
		::ShowWindow,
		::UpdateWindow,
		::GetMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::PostQuitMessage,
		::SendMessageW,
		::MessageBoxW,
		::GetLastError,
		::GetStockObject,
		::GetWindowLongPtrW,
		::SetWindowLongPtrW,
		::LoadCursorW,
		::AdjustWindowRectEx,
		::InitCommonControlsEx,
		::LoadLibraryW,
		::CreateSolidBrush,
		::DeleteObject,
		::FillRect,
		::GetClientRect
		;

	// === HIWORD/LOWORD wrappers ===
	auto GetHighWord(WPARAM wParam) noexcept -> WORD { return HIWORD(wParam); }
	auto GetLowWord(WPARAM wParam) noexcept -> WORD { return LOWORD(wParam); }

	// === Window and control styles ===
	namespace Styles
	{
		constexpr auto OverlappedWindow = WS_OVERLAPPEDWINDOW;
		constexpr auto Child = WS_CHILD;
		constexpr auto Visible = WS_VISIBLE;
		constexpr auto Border = WS_BORDER;
		constexpr auto PushButton = BS_DEFPUSHBUTTON;
		constexpr auto AutoCheckBox = BS_AUTOCHECKBOX;
		constexpr auto AutoRadioButton = BS_AUTORADIOBUTTON;
		constexpr auto GroupBox = BS_GROUPBOX;
		constexpr auto EditAutoHScroll = ES_AUTOHSCROLL;
		constexpr auto StaticLeft = SS_LEFT;
		constexpr auto StaticCenter = SS_CENTER;
		constexpr auto StaticRight = SS_RIGHT;
		constexpr auto EditCenter = ES_CENTER;
		constexpr auto EditRight = ES_RIGHT;
		constexpr auto ButtonLeft = BS_LEFT;
		constexpr auto ButtonCenter = BS_CENTER;
		constexpr auto ButtonRight = BS_RIGHT;
		constexpr auto ListBoxStandard = LBS_STANDARD;
		constexpr auto ComboBoxDropDownList = CBS_DROPDOWNLIST;
		constexpr auto ListViewReport = LVS_REPORT;
		constexpr auto ListViewShowSelAlways = LVS_SHOWSELALWAYS;
		constexpr auto TreeViewHasButtons = TVS_HASBUTTONS;
		constexpr auto TreeViewHasLines = TVS_HASLINES;
		constexpr auto TreeViewLinesAtRoot = TVS_LINESATROOT;
		constexpr auto EditMultiLine = ES_MULTILINE;
		constexpr auto EditAutoVScroll = ES_AUTOVSCROLL;
	}

	// === Class styles ===
	constexpr auto Cs_HRedraw = CS_HREDRAW;
	constexpr auto Cs_VRedraw = CS_VREDRAW;

	// === Messages ===
	namespace Messages
	{
		constexpr auto Command = WM_COMMAND;
		constexpr auto Notify = WM_NOTIFY;
		constexpr auto Destroy = WM_DESTROY;
		constexpr auto SetFont = WM_SETFONT;
		constexpr auto EraseBkgnd = WM_ERASEBKGND;
	}

	// === Notifications (WM_COMMAND) ===
	namespace Notifications
	{
		constexpr auto ButtonClicked = BN_CLICKED;
		constexpr auto ButtonDoubleClicked = BN_DBLCLK;
		constexpr auto EditChange = EN_CHANGE;
		constexpr auto EditSetFocus = EN_SETFOCUS;
		constexpr auto EditKillFocus = EN_KILLFOCUS;
		constexpr auto ListBoxSelChange = LBN_SELCHANGE;
		constexpr auto ListBoxDoubleClick = LBN_DBLCLK;
		constexpr auto ListBoxSetFocus = LBN_SETFOCUS;
		constexpr auto ListBoxKillFocus = LBN_KILLFOCUS;
		constexpr auto ComboBoxSelChange = CBN_SELCHANGE;
		constexpr auto ComboBoxSetFocus = CBN_SETFOCUS;
		constexpr auto ComboBoxKillFocus = CBN_KILLFOCUS;
	}

	// === Notifications (WM_NOTIFY) ===
	using ::NMHDR;
	namespace NotifyCodes
	{
		constexpr auto DoubleClick = NM_DBLCLK;
		constexpr auto DateTimeChange = DTN_DATETIMECHANGE;
		constexpr auto TreeViewSelChanged = TVN_SELCHANGEDW;
		constexpr auto ListViewItemChanged = LVN_ITEMCHANGED;
		constexpr auto TabSelChange = TCN_SELCHANGE;
	}

	// === Button messages and states ===
	namespace Button
	{
		constexpr auto GetCheck = BM_GETCHECK;
		constexpr auto Checked = BST_CHECKED;
	}

	// === Control class names ===
	namespace Controls
	{
		constexpr auto Button = WC_BUTTON;
		constexpr auto Static = WC_STATIC;
		constexpr auto Edit = WC_EDIT;
		constexpr auto ListBox = WC_LISTBOX;
		constexpr auto ComboBox = WC_COMBOBOX;
		constexpr auto Progress = PROGRESS_CLASS;
		constexpr auto TrackBar = TRACKBAR_CLASS;
		constexpr auto DateTimePick = DATETIMEPICK_CLASS;
		constexpr auto Tab = WC_TABCONTROL;
		constexpr auto ListView = WC_LISTVIEW;
		constexpr auto TreeView = WC_TREEVIEW;
		constexpr auto UpDown = UPDOWN_CLASS;
		constexpr auto RichEdit = MSFTEDIT_CLASS;
	}

	// === System constants ===
	constexpr auto Gwlp_UserData = GWLP_USERDATA;
	constexpr auto Cw_UseDefault = CW_USEDEFAULT;
	constexpr auto Sw_ShowDefault = SW_SHOWDEFAULT;
	constexpr auto DefaultGuiFont = DEFAULT_GUI_FONT;
	constexpr auto ColorWindow = COLOR_WINDOW;
	constexpr auto ColorBtnFace = COLOR_BTNFACE;

	// === Cursors ===
	namespace Cursors
	{
		const auto Arrow = IDC_ARROW;
	}

	// === Common controls ===
	constexpr auto Icc_StandardClasses = ICC_STANDARD_CLASSES;

	// === MessageBox flags ===
	constexpr auto Mb_Ok = MB_OK;
	constexpr auto Mb_IconInformation = MB_ICONINFORMATION;
	constexpr auto Mb_IconError = MB_ICONERROR;
}
