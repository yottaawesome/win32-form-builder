module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Commdlg.h>

export module designer:properties;
import std;
import formbuilder;
import :state;
import :helpers;

namespace Designer
{

void SetPropertyGroupVisibility(HWND panel, const UINT ids[], int count, int show)
{
    for (int i = 0; i < count; ++i)
    {
        auto edit = GetDlgItem(panel, ids[i]);
        if (edit) ShowWindow(edit, show);
        auto label = GetDlgItem(panel, ids[i] + IDL_OFFSET);
        if (label) ShowWindow(label, show);
    }
}

export void UpdatePropertyPanel(DesignState& state)
{
    if (!state.propertyHwnd) return;
    state.updatingProperties = true;

    auto panel = state.propertyHwnd;
    bool hasSel = state.selectedIndex >= 0 &&
        state.selectedIndex < static_cast<int>(state.entries.size());

    constexpr UINT ctrlIds[] = {
        IDC_PROP_TYPE, IDC_PROP_TEXT, IDC_PROP_ID,
        IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H, IDC_PROP_ONCLICK
    };
    constexpr UINT formIds[] = {
        IDC_PROP_FORM_TITLE, IDC_PROP_FORM_WIDTH,
        IDC_PROP_FORM_HEIGHT, IDC_PROP_FORM_BGCOLOR
    };

    SetPropertyGroupVisibility(panel, ctrlIds, 8, hasSel ? SW_SHOW : SW_HIDE);
    SetPropertyGroupVisibility(panel, formIds, 4, hasSel ? SW_HIDE : SW_SHOW);

    auto bgBtn = GetDlgItem(panel, IDC_PROP_FORM_BGCOLOR_BTN);
    if (bgBtn) ShowWindow(bgBtn, hasSel ? SW_HIDE : SW_SHOW);

    if (hasSel)
    {
        auto& ctrl = *state.entries[state.selectedIndex].control;

        SetDlgItemTextW(panel, IDC_PROP_TYPE, ControlTypeDisplayName(ctrl.type));
        SetDlgItemTextW(panel, IDC_PROP_TEXT, ctrl.text.c_str());
        SetDlgItemInt(panel, IDC_PROP_ID, static_cast<UINT>(ctrl.id), FALSE);
        SetDlgItemInt(panel, IDC_PROP_X, ctrl.rect.x, TRUE);
        SetDlgItemInt(panel, IDC_PROP_Y, ctrl.rect.y, TRUE);
        SetDlgItemInt(panel, IDC_PROP_W, ctrl.rect.width, TRUE);
        SetDlgItemInt(panel, IDC_PROP_H, ctrl.rect.height, TRUE);

        auto onClick = std::wstring(ctrl.onClick.begin(), ctrl.onClick.end());
        SetDlgItemTextW(panel, IDC_PROP_ONCLICK, onClick.c_str());

        UINT editableIds[] = { IDC_PROP_TEXT, IDC_PROP_ID,
            IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H, IDC_PROP_ONCLICK };
        for (auto id : editableIds)
            EnableWindow(GetDlgItem(panel, id), TRUE);
    }
    else
    {
        SetDlgItemTextW(panel, IDC_PROP_FORM_TITLE, state.form.title.c_str());
        SetDlgItemInt(panel, IDC_PROP_FORM_WIDTH, state.form.width, FALSE);
        SetDlgItemInt(panel, IDC_PROP_FORM_HEIGHT, state.form.height, FALSE);
        SetDlgItemTextW(panel, IDC_PROP_FORM_BGCOLOR,
            ColorRefToHex(state.form.backgroundColor).c_str());
    }

    state.updatingProperties = false;
}

void ApplyPropertyChange(DesignState& state, UINT controlId)
{
    if (state.selectedIndex < 0 ||
        state.selectedIndex >= static_cast<int>(state.entries.size()))
        return;

    auto& entry = state.entries[state.selectedIndex];
    auto& ctrl = *entry.control;
    auto panel = state.propertyHwnd;

    switch (controlId)
    {
    case IDC_PROP_TEXT:
    {
        wchar_t buf[512] = {};
        GetDlgItemTextW(panel, IDC_PROP_TEXT, buf, 512);
        ctrl.text = buf;
        SetWindowTextW(entry.hwnd, ctrl.text.c_str());
        break;
    }
    case IDC_PROP_ID:
    {
        BOOL ok = FALSE;
        auto val = GetDlgItemInt(panel, IDC_PROP_ID, &ok, FALSE);
        if (ok) ctrl.id = static_cast<int>(val);
        break;
    }
    case IDC_PROP_X:
    {
        BOOL ok = FALSE;
        auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_X, &ok, TRUE));
        if (ok) ctrl.rect.x = val;
        MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
            ctrl.rect.width, ctrl.rect.height, TRUE);
        InvalidateRect(state.canvasHwnd, nullptr, TRUE);
        break;
    }
    case IDC_PROP_Y:
    {
        BOOL ok = FALSE;
        auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_Y, &ok, TRUE));
        if (ok) ctrl.rect.y = val;
        MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
            ctrl.rect.width, ctrl.rect.height, TRUE);
        InvalidateRect(state.canvasHwnd, nullptr, TRUE);
        break;
    }
    case IDC_PROP_W:
    {
        BOOL ok = FALSE;
        auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_W, &ok, FALSE));
        if (ok && val >= MIN_CONTROL_SIZE) ctrl.rect.width = val;
        MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
            ctrl.rect.width, ctrl.rect.height, TRUE);
        InvalidateRect(state.canvasHwnd, nullptr, TRUE);
        break;
    }
    case IDC_PROP_H:
    {
        BOOL ok = FALSE;
        auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_H, &ok, FALSE));
        if (ok && val >= MIN_CONTROL_SIZE) ctrl.rect.height = val;
        MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
            ctrl.rect.width, ctrl.rect.height, TRUE);
        InvalidateRect(state.canvasHwnd, nullptr, TRUE);
        break;
    }
    case IDC_PROP_ONCLICK:
    {
        wchar_t buf[256] = {};
        GetDlgItemTextW(panel, IDC_PROP_ONCLICK, buf, 256);
        ctrl.onClick = std::string(buf, buf + wcslen(buf));
        break;
    }
    default:
        return;
    }

    MarkDirty(state);
}

void ApplyFormPropertyChange(DesignState& state, UINT controlId)
{
    auto panel = state.propertyHwnd;

    switch (controlId)
    {
    case IDC_PROP_FORM_TITLE:
    {
        wchar_t buf[256] = {};
        GetDlgItemTextW(panel, IDC_PROP_FORM_TITLE, buf, 256);
        state.form.title = buf;
        break;
    }
    case IDC_PROP_FORM_WIDTH:
    {
        BOOL ok = FALSE;
        auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_FORM_WIDTH, &ok, FALSE));
        if (ok && val > 0) state.form.width = val;
        break;
    }
    case IDC_PROP_FORM_HEIGHT:
    {
        BOOL ok = FALSE;
        auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_FORM_HEIGHT, &ok, FALSE));
        if (ok && val > 0) state.form.height = val;
        break;
    }
    case IDC_PROP_FORM_BGCOLOR:
    {
        wchar_t buf[16] = {};
        GetDlgItemTextW(panel, IDC_PROP_FORM_BGCOLOR, buf, 16);
        auto hex = std::wstring(buf);
        state.form.backgroundColor = hex.empty() ? -1 : HexToColorRef(hex);
        InvalidateRect(state.canvasHwnd, nullptr, TRUE);
        break;
    }
    default:
        return;
    }

    MarkDirty(state);
}

export void CreatePropertyControls(DesignState& state)
{
    auto parent = state.propertyHwnd;
    auto hInst = state.hInstance;
    auto font = reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT));

    auto header = CreateWindowExW(0, L"STATIC", L"Properties",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        5, 5, 210, 20, parent, nullptr, hInst, nullptr);
    SendMessageW(header, WM_SETFONT, font, TRUE);

    // Control property rows (visible when a control is selected).
    struct PropRow { const wchar_t* label; UINT editId; DWORD extraStyle; };

    PropRow ctrlRows[] = {
        { L"Type:",    IDC_PROP_TYPE,    ES_READONLY },
        { L"Text:",    IDC_PROP_TEXT,    ES_AUTOHSCROLL },
        { L"ID:",      IDC_PROP_ID,     ES_AUTOHSCROLL },
        { L"X:",       IDC_PROP_X,      ES_AUTOHSCROLL },
        { L"Y:",       IDC_PROP_Y,      ES_AUTOHSCROLL },
        { L"Width:",   IDC_PROP_W,      ES_AUTOHSCROLL },
        { L"Height:",  IDC_PROP_H,      ES_AUTOHSCROLL },
        { L"onClick:", IDC_PROP_ONCLICK, ES_AUTOHSCROLL },
    };

    int y = 30;
    for (auto& row : ctrlRows)
    {
        auto lbl = CreateWindowExW(0, L"STATIC", row.label,
            WS_CHILD | SS_RIGHT,
            5, y + 2, 55, 18, parent,
            reinterpret_cast<HMENU>(static_cast<UINT_PTR>(row.editId + IDL_OFFSET)),
            hInst, nullptr);
        SendMessageW(lbl, WM_SETFONT, font, TRUE);

        auto edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_TABSTOP | row.extraStyle,
            65, y, 150, 22, parent,
            reinterpret_cast<HMENU>(static_cast<UINT_PTR>(row.editId)),
            hInst, nullptr);
        SendMessageW(edit, WM_SETFONT, font, TRUE);
        EnableWindow(edit, FALSE);

        y += 26;
    }

    // Form property rows (visible when no control is selected).
    PropRow formRows[] = {
        { L"Title:",   IDC_PROP_FORM_TITLE,  ES_AUTOHSCROLL },
        { L"Width:",   IDC_PROP_FORM_WIDTH,  ES_AUTOHSCROLL },
        { L"Height:",  IDC_PROP_FORM_HEIGHT, ES_AUTOHSCROLL },
        { L"BgColor:", IDC_PROP_FORM_BGCOLOR, ES_AUTOHSCROLL },
    };

    y = 30;
    for (auto& row : formRows)
    {
        auto lbl = CreateWindowExW(0, L"STATIC", row.label,
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            5, y + 2, 55, 18, parent,
            reinterpret_cast<HMENU>(static_cast<UINT_PTR>(row.editId + IDL_OFFSET)),
            hInst, nullptr);
        SendMessageW(lbl, WM_SETFONT, font, TRUE);

        int editW = (row.editId == IDC_PROP_FORM_BGCOLOR) ? 105 : 150;
        auto edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | row.extraStyle,
            65, y, editW, 22, parent,
            reinterpret_cast<HMENU>(static_cast<UINT_PTR>(row.editId)),
            hInst, nullptr);
        SendMessageW(edit, WM_SETFONT, font, TRUE);

        y += 26;
    }

    // Color picker button next to BgColor.
    auto bgBtn = CreateWindowExW(0, L"BUTTON", L"...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        175, 30 + 3 * 26, 40, 22, parent,
        reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_PROP_FORM_BGCOLOR_BTN)),
        hInst, nullptr);
    SendMessageW(bgBtn, WM_SETFONT, font, TRUE);
}

COLORREF g_customColors[16] = {};

export LRESULT CALLBACK PropertyPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto* state = reinterpret_cast<DesignState*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_COMMAND:
    {
        if (!state || state->updatingProperties) break;

        UINT code = HIWORD(wParam);
        UINT id = LOWORD(wParam);

        // Color picker button.
        if (id == IDC_PROP_FORM_BGCOLOR_BTN && code == BN_CLICKED)
        {
            COLORREF initial = (state->form.backgroundColor != -1)
                ? static_cast<COLORREF>(state->form.backgroundColor)
                : GetSysColor(COLOR_BTNFACE);

            CHOOSECOLORW cc = {
                .lStructSize = sizeof(CHOOSECOLORW),
                .hwndOwner = state->surfaceHwnd,
                .rgbResult = initial,
                .lpCustColors = g_customColors,
                .Flags = CC_FULLOPEN | CC_RGBINIT,
            };

            if (ChooseColorW(&cc))
            {
                state->form.backgroundColor = static_cast<int>(cc.rgbResult);
                UpdatePropertyPanel(*state);
                InvalidateRect(state->canvasHwnd, nullptr, TRUE);
                MarkDirty(*state);
            }
            return 0;
        }

        // Property edits (apply on focus loss).
        if (code == EN_KILLFOCUS)
        {
            if (id >= IDC_PROP_TYPE && id <= IDC_PROP_ONCLICK)
                ApplyPropertyChange(*state, id);
            else if (id >= IDC_PROP_FORM_TITLE && id <= IDC_PROP_FORM_BGCOLOR)
                ApplyFormPropertyChange(*state, id);
            return 0;
        }
        break;
    }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

}
