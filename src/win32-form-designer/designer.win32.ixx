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
    ::SetBkMode
    ;

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
    constexpr auto Close        = WM_CLOSE;
    constexpr auto NcDestroy    = WM_NCDESTROY;
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
    constexpr auto Delete = VK_DELETE;
    constexpr auto Escape = VK_ESCAPE;
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
    constexpr auto ListBoxNotify            = LBS_NOTIFY;
    constexpr auto ListBoxNoIntegralHeight  = LBS_NOINTEGRALHEIGHT;
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
}

// === Menu flags ===
namespace Menu
{
    constexpr auto String    = MF_STRING;
    constexpr auto Separator = MF_SEPARATOR;
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

// === Miscellaneous ===
constexpr auto MaxPath = MAX_PATH;

}
