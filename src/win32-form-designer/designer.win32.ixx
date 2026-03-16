module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <Commdlg.h>
#include <windowsx.h>

export module designer:win32;

export namespace Win32
{

// === Additional types (beyond formbuilder's Win32) ===
using
    ::UINT_PTR,
    ::DWORD_PTR,
    ::SIZE,
    ::POINT,
    ::PAINTSTRUCT,
    ::COLORREF,
    ::OPENFILENAMEW,
    ::CHOOSECOLORW,
    ::HACCEL,
    ::ACCEL,
    ::LPCWSTR
    ;

// === Additional functions ===
using
    ::DefSubclassProc,
    ::SetWindowSubclass,
    ::MoveWindow,
    ::InvalidateRect,
    ::SetWindowTextW,
    ::GetDlgItem,
    ::SetDlgItemTextW,
    ::SetDlgItemInt,
    ::GetDlgItemTextW,
    ::GetDlgItemInt,
    ::EnableWindow,
    ::SetCapture,
    ::ReleaseCapture,
    ::SetFocus,
    ::GetDC,
    ::ReleaseDC,
    ::BeginPaint,
    ::EndPaint,
    ::CreateMenu,
    ::CreatePopupMenu,
    ::AppendMenuW,
    ::CreateAcceleratorTableW,
    ::DestroyAcceleratorTable,
    ::TranslateAcceleratorW,
    ::GetSaveFileNameW,
    ::GetOpenFileNameW,
    ::ChooseColorW,
    ::GetSysColor,
    ::SetCursor,
    ::CreatePen,
    ::SelectObject,
    ::MoveToEx,
    ::LineTo,
    ::SetBkMode,
    ::GetKeyState,
    ::SetScrollInfo,
    ::GetScrollInfo,
    ::ScrollWindowEx,
    ::SetWindowPos,
    ::GetWindowRect,
    ::TrackPopupMenu,
    ::ClientToScreen,
    ::DestroyMenu
    ;

const auto HwndBottom = HWND_BOTTOM;
const auto HwndTop    = HWND_TOP;

namespace Swp
{
    constexpr auto NoMove     = SWP_NOMOVE;
    constexpr auto NoSize     = SWP_NOSIZE;
    constexpr auto NoActivate = SWP_NOACTIVATE;
}

// === Scroll types ===
using ::SCROLLINFO;

// === Pen styles ===
namespace PenStyles
{
    constexpr auto Dot = PS_DOT;
}

// === Background modes ===
constexpr auto Bk_Transparent = TRANSPARENT;

// === Macro wrappers ===
auto GetXParam(LPARAM lp) noexcept -> int { return GET_X_LPARAM(lp); }
auto GetYParam(LPARAM lp) noexcept -> int { return GET_Y_LPARAM(lp); }
auto MakeRgb(int r, int g, int b) noexcept -> COLORREF { return RGB(r, g, b); }

// === Additional messages ===
namespace Messages
{
    constexpr auto NcHitTest    = WM_NCHITTEST;
    constexpr auto Paint        = WM_PAINT;
    constexpr auto LButtonDown  = WM_LBUTTONDOWN;
    constexpr auto MouseMove    = WM_MOUSEMOVE;
    constexpr auto LButtonUp    = WM_LBUTTONUP;
    constexpr auto SetCursorMsg = WM_SETCURSOR;
    constexpr auto KeyDown      = WM_KEYDOWN;
    constexpr auto Size         = WM_SIZE;
    constexpr auto VScroll      = WM_VSCROLL;
    constexpr auto MouseWheel   = WM_MOUSEWHEEL;
    constexpr auto Close        = WM_CLOSE;
    constexpr auto NcDestroy    = WM_NCDESTROY;
    constexpr auto RButtonUp    = WM_RBUTTONUP;
}

namespace TrackPopup
{
    constexpr auto LeftAlign   = TPM_LEFTALIGN;
    constexpr auto TopAlign    = TPM_TOPALIGN;
    constexpr auto ReturnCmd   = TPM_RETURNCMD;
    constexpr auto RightButton = TPM_RIGHTBUTTON;
}

// === Additional notifications ===
namespace Notifications
{
    constexpr auto EditKillFocus    = EN_KILLFOCUS;
    constexpr auto ListBoxSelChange = LBN_SELCHANGE;
}

// === Hit test values ===
namespace HitTestValues
{
    constexpr auto Transparent = HTTRANSPARENT;
    constexpr auto Client      = HTCLIENT;
}

// === Virtual keys ===
namespace Keys
{
    constexpr auto Delete  = VK_DELETE;
    constexpr auto Escape  = VK_ESCAPE;
    constexpr auto Control = VK_CONTROL;
}

// === Show window commands ===
constexpr auto Sw_Show = SW_SHOW;
constexpr auto Sw_Hide = SW_HIDE;

// === Additional styles ===
namespace Styles
{
    constexpr auto StaticRight              = SS_RIGHT;
    constexpr auto EditReadOnly             = ES_READONLY;
    constexpr auto ButtonPush               = BS_PUSHBUTTON;
    constexpr auto TabStop                  = WS_TABSTOP;
    constexpr auto VScroll                  = WS_VSCROLL;
    constexpr auto ListBoxNotify            = LBS_NOTIFY;
    constexpr auto ListBoxNoIntegralHeight  = LBS_NOINTEGRALHEIGHT;
    constexpr auto ThickFrame               = WS_THICKFRAME;
    constexpr auto MaximizeBox              = WS_MAXIMIZEBOX;
}

// === Extended styles ===
namespace ExStyles
{
    constexpr auto ClientEdge = WS_EX_CLIENTEDGE;
}

// === ListBox messages ===
namespace ListBox
{
    constexpr auto GetCurSel = LB_GETCURSEL;
    constexpr auto SetCurSel = LB_SETCURSEL;
    constexpr auto AddString = LB_ADDSTRING;
    constexpr auto ResetContent = LB_RESETCONTENT;
}

// === Menu flags ===
namespace Menu
{
    constexpr auto String    = MF_STRING;
    constexpr auto Separator = MF_SEPARATOR;
    constexpr auto Grayed    = MF_GRAYED;
    constexpr auto Popup     = MF_POPUP;
}

// === MessageBox flags ===
constexpr auto Mb_YesNoCancel  = MB_YESNOCANCEL;
constexpr auto Mb_IconQuestion = MB_ICONQUESTION;
constexpr auto Id_Cancel       = IDCANCEL;
constexpr auto Id_Yes          = IDYES;

// === File dialog flags ===
namespace FileDialog
{
    constexpr auto OverwritePrompt = OFN_OVERWRITEPROMPT;
    constexpr auto PathMustExist   = OFN_PATHMUSTEXIST;
    constexpr auto FileMustExist   = OFN_FILEMUSTEXIST;
}

// === Color dialog flags ===
namespace ColorDialog
{
    constexpr auto FullOpen = CC_FULLOPEN;
    constexpr auto RgbInit  = CC_RGBINIT;
}

// === Accelerator flags ===
namespace Accel
{
    constexpr auto Control = FCONTROL;
    constexpr auto Shift   = FSHIFT;
    constexpr auto VirtKey = FVIRTKEY;
}

// === Additional cursors ===
namespace Cursors
{
    const auto Cross    = IDC_CROSS;
    const auto SizeNWSE = IDC_SIZENWSE;
    const auto SizeNESW = IDC_SIZENESW;
    const auto SizeNS   = IDC_SIZENS;
    const auto SizeWE   = IDC_SIZEWE;
}

// === Scroll bar constants ===
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

// === Macro wrappers ===
auto GetWheelDelta(WPARAM wParam) noexcept -> short { return GET_WHEEL_DELTA_WPARAM(wParam); }

// === Miscellaneous ===
constexpr auto MaxPath = MAX_PATH;

}
