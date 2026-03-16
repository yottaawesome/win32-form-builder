export module designer:properties;
import std;
import formbuilder;
import :win32;
import :state;
import :helpers;

namespace Designer
{

void UpdateScrollRange(DesignState& state);
void ScrollPropertyPanel(DesignState& state, int newPos);
void ResetPropertyScroll(DesignState& state);

void SetPropertyGroupVisibility(Win32::HWND panel, const Win32::UINT ids[], int count, int show)
{
    for (int i = 0; i < count; ++i)
    {
        auto edit = Win32::GetDlgItem(panel, ids[i]);
        if (edit) Win32::ShowWindow(edit, show);
        auto label = Win32::GetDlgItem(panel, ids[i] + IDL_OFFSET);
        if (label) Win32::ShowWindow(label, show);
    }
}

export void UpdatePropertyPanel(DesignState& state)
{
    if (!state.propertyHwnd) return;
    state.updatingProperties = true;

    auto panel = state.propertyHwnd;
    int sel = SingleSelection(state);
    bool hasSel = sel >= 0 && sel < static_cast<int>(state.entries.size());

    constexpr Win32::UINT ctrlIds[] = {
        IDC_PROP_TYPE, IDC_PROP_TEXT, IDC_PROP_ID,
        IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H,
        IDC_PROP_ONCLICK, IDC_PROP_ONCHANGE, IDC_PROP_ONDBLCLICK, IDC_PROP_ONSELCHANGE,
        IDC_PROP_ONFOCUS, IDC_PROP_ONBLUR, IDC_PROP_ONCHECK, IDC_PROP_TABINDEX,
        IDC_PROP_TEXTALIGN
    };
    constexpr Win32::UINT formIds[] = {
        IDC_PROP_FORM_TITLE, IDC_PROP_FORM_WIDTH,
        IDC_PROP_FORM_HEIGHT, IDC_PROP_FORM_BGCOLOR
    };

    SetPropertyGroupVisibility(panel, ctrlIds, 16, hasSel ? Win32::Sw_Show : Win32::Sw_Hide);
    SetPropertyGroupVisibility(panel, formIds, 4, hasSel ? Win32::Sw_Hide : Win32::Sw_Show);

    auto bgBtn = Win32::GetDlgItem(panel, IDC_PROP_FORM_BGCOLOR_BTN);
    if (bgBtn) Win32::ShowWindow(bgBtn, hasSel ? Win32::Sw_Hide : Win32::Sw_Show);

    ResetPropertyScroll(state);
    UpdateScrollRange(state);

    if (hasSel)
    {
        auto& ctrl = *state.entries[sel].control;

        Win32::SetDlgItemTextW(panel, IDC_PROP_TYPE, ControlTypeDisplayName(ctrl.type));
        Win32::SetDlgItemTextW(panel, IDC_PROP_TEXT, ctrl.text.c_str());
        Win32::SetDlgItemInt(panel, IDC_PROP_ID, static_cast<Win32::UINT>(ctrl.id), false);
        Win32::SetDlgItemInt(panel, IDC_PROP_X, ctrl.rect.x, true);
        Win32::SetDlgItemInt(panel, IDC_PROP_Y, ctrl.rect.y, true);
        Win32::SetDlgItemInt(panel, IDC_PROP_W, ctrl.rect.width, true);
        Win32::SetDlgItemInt(panel, IDC_PROP_H, ctrl.rect.height, true);

        auto onClick = std::wstring(ctrl.onClick.begin(), ctrl.onClick.end());
        Win32::SetDlgItemTextW(panel, IDC_PROP_ONCLICK, onClick.c_str());

        auto onChange = std::wstring(ctrl.onChange.begin(), ctrl.onChange.end());
        Win32::SetDlgItemTextW(panel, IDC_PROP_ONCHANGE, onChange.c_str());

        auto onDblClick = std::wstring(ctrl.onDoubleClick.begin(), ctrl.onDoubleClick.end());
        Win32::SetDlgItemTextW(panel, IDC_PROP_ONDBLCLICK, onDblClick.c_str());

        auto onSelChange = std::wstring(ctrl.onSelectionChange.begin(), ctrl.onSelectionChange.end());
        Win32::SetDlgItemTextW(panel, IDC_PROP_ONSELCHANGE, onSelChange.c_str());

        auto onFocus = std::wstring(ctrl.onFocus.begin(), ctrl.onFocus.end());
        Win32::SetDlgItemTextW(panel, IDC_PROP_ONFOCUS, onFocus.c_str());

        auto onBlur = std::wstring(ctrl.onBlur.begin(), ctrl.onBlur.end());
        Win32::SetDlgItemTextW(panel, IDC_PROP_ONBLUR, onBlur.c_str());

        auto onCheck = std::wstring(ctrl.onCheck.begin(), ctrl.onCheck.end());
        Win32::SetDlgItemTextW(panel, IDC_PROP_ONCHECK, onCheck.c_str());

        Win32::SetDlgItemInt(panel, IDC_PROP_TABINDEX, static_cast<Win32::UINT>(ctrl.tabIndex), true);

        // Set text alignment dropdown.
        auto alignCombo = Win32::GetDlgItem(panel, IDC_PROP_TEXTALIGN);
        if (alignCombo)
            Win32::SendMessageW(alignCombo, Win32::ComboBox::SetCurSel,
                static_cast<Win32::WPARAM>(ctrl.textAlign), 0);

        Win32::UINT editableIds[] = { IDC_PROP_TEXT, IDC_PROP_ID,
            IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H,
            IDC_PROP_ONCLICK, IDC_PROP_ONCHANGE, IDC_PROP_ONDBLCLICK, IDC_PROP_ONSELCHANGE,
            IDC_PROP_ONFOCUS, IDC_PROP_ONBLUR, IDC_PROP_ONCHECK, IDC_PROP_TABINDEX };
        for (auto id : editableIds)
            Win32::EnableWindow(Win32::GetDlgItem(panel, id), true);
    }
    else
    {
        Win32::SetDlgItemTextW(panel, IDC_PROP_FORM_TITLE, state.form.title.c_str());
        Win32::SetDlgItemInt(panel, IDC_PROP_FORM_WIDTH, state.form.width, false);
        Win32::SetDlgItemInt(panel, IDC_PROP_FORM_HEIGHT, state.form.height, false);
        Win32::SetDlgItemTextW(panel, IDC_PROP_FORM_BGCOLOR,
            ColorRefToHex(state.form.backgroundColor).c_str());
    }

    state.updatingProperties = false;
}

void ApplyPropertyChange(DesignState& state, Win32::UINT controlId)
{
    int sel = SingleSelection(state);
    if (sel < 0 || sel >= static_cast<int>(state.entries.size()))
        return;

    PushUndo(state);

    auto& entry = state.entries[sel];
    auto& ctrl = *entry.control;
    auto panel = state.propertyHwnd;

    switch (controlId)
    {
    case IDC_PROP_TEXT:
    {
        wchar_t buf[512] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_TEXT, buf, 512);
        ctrl.text = buf;
        Win32::SetWindowTextW(entry.hwnd, ctrl.text.c_str());
        break;
    }
    case IDC_PROP_ID:
    {
        Win32::BOOL ok = false;
        auto val = Win32::GetDlgItemInt(panel, IDC_PROP_ID, &ok, false);
        if (ok) ctrl.id = static_cast<int>(val);
        break;
    }
    case IDC_PROP_X:
    {
        Win32::BOOL ok = false;
        auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_X, &ok, true));
        if (ok) ctrl.rect.x = val;
        Win32::MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
            ctrl.rect.width, ctrl.rect.height, true);
        Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
        break;
    }
    case IDC_PROP_Y:
    {
        Win32::BOOL ok = false;
        auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_Y, &ok, true));
        if (ok) ctrl.rect.y = val;
        Win32::MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
            ctrl.rect.width, ctrl.rect.height, true);
        Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
        break;
    }
    case IDC_PROP_W:
    {
        Win32::BOOL ok = false;
        auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_W, &ok, false));
        if (ok && val >= MIN_CONTROL_SIZE) ctrl.rect.width = val;
        Win32::MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
            ctrl.rect.width, ctrl.rect.height, true);
        Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
        break;
    }
    case IDC_PROP_H:
    {
        Win32::BOOL ok = false;
        auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_H, &ok, false));
        if (ok && val >= MIN_CONTROL_SIZE) ctrl.rect.height = val;
        Win32::MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
            ctrl.rect.width, ctrl.rect.height, true);
        Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
        break;
    }
    case IDC_PROP_ONCLICK:
    {
        wchar_t buf[256] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_ONCLICK, buf, 256);
        ctrl.onClick = std::string(buf, buf + std::wcslen(buf));
        break;
    }
    case IDC_PROP_ONCHANGE:
    {
        wchar_t buf[256] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_ONCHANGE, buf, 256);
        ctrl.onChange = std::string(buf, buf + std::wcslen(buf));
        break;
    }
    case IDC_PROP_ONDBLCLICK:
    {
        wchar_t buf[256] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_ONDBLCLICK, buf, 256);
        ctrl.onDoubleClick = std::string(buf, buf + std::wcslen(buf));
        break;
    }
    case IDC_PROP_ONSELCHANGE:
    {
        wchar_t buf[256] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_ONSELCHANGE, buf, 256);
        ctrl.onSelectionChange = std::string(buf, buf + std::wcslen(buf));
        break;
    }
    case IDC_PROP_ONFOCUS:
    {
        wchar_t buf[256] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_ONFOCUS, buf, 256);
        ctrl.onFocus = std::string(buf, buf + std::wcslen(buf));
        break;
    }
    case IDC_PROP_ONBLUR:
    {
        wchar_t buf[256] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_ONBLUR, buf, 256);
        ctrl.onBlur = std::string(buf, buf + std::wcslen(buf));
        break;
    }
    case IDC_PROP_ONCHECK:
    {
        wchar_t buf[256] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_ONCHECK, buf, 256);
        ctrl.onCheck = std::string(buf, buf + std::wcslen(buf));
        break;
    }
    case IDC_PROP_TABINDEX:
    {
        Win32::BOOL ok = false;
        auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_TABINDEX, &ok, true));
        if (ok) ctrl.tabIndex = val;
        break;
    }
    case IDC_PROP_TEXTALIGN:
    {
        auto combo = Win32::GetDlgItem(panel, IDC_PROP_TEXTALIGN);
        auto sel = static_cast<int>(Win32::SendMessageW(combo, Win32::ComboBox::GetCurSel, 0, 0));
        if (sel >= 0 && sel <= 2)
        {
            ctrl.textAlign = static_cast<FormDesigner::TextAlign>(sel);
            // Recreate the control HWND since style bits are set at creation.
            RebuildSingleControl(state, entry);
        }
        break;
    }
    default:
        return;
    }

    MarkDirty(state);
}

void ApplyFormPropertyChange(DesignState& state, Win32::UINT controlId)
{
    PushUndo(state);
    auto panel = state.propertyHwnd;

    switch (controlId)
    {
    case IDC_PROP_FORM_TITLE:
    {
        wchar_t buf[256] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_FORM_TITLE, buf, 256);
        state.form.title = buf;
        break;
    }
    case IDC_PROP_FORM_WIDTH:
    {
        Win32::BOOL ok = false;
        auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_FORM_WIDTH, &ok, false));
        if (ok && val > 0) state.form.width = val;
        break;
    }
    case IDC_PROP_FORM_HEIGHT:
    {
        Win32::BOOL ok = false;
        auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_FORM_HEIGHT, &ok, false));
        if (ok && val > 0) state.form.height = val;
        break;
    }
    case IDC_PROP_FORM_BGCOLOR:
    {
        wchar_t buf[16] = {};
        Win32::GetDlgItemTextW(panel, IDC_PROP_FORM_BGCOLOR, buf, 16);
        auto hex = std::wstring(buf);
        state.form.backgroundColor = hex.empty() ? -1 : HexToColorRef(hex);
        Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
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
    auto font = reinterpret_cast<Win32::WPARAM>(
        Win32::GetStockObject(Win32::DefaultGuiFont));

    auto header = Win32::CreateWindowExW(0, L"STATIC", L"Properties",
        Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticLeft,
        5, 5, 210, 20, parent, nullptr, hInst, nullptr);
    Win32::SendMessageW(header, Win32::Messages::SetFont, font, true);

    // Control property rows (visible when a control is selected).
    struct PropRow { const wchar_t* label; Win32::UINT editId; Win32::DWORD extraStyle; };

    PropRow ctrlRows[] = {
        { L"Type:",    IDC_PROP_TYPE,    Win32::Styles::EditReadOnly },
        { L"Text:",    IDC_PROP_TEXT,    Win32::Styles::EditAutoHScroll },
        { L"ID:",      IDC_PROP_ID,     Win32::Styles::EditAutoHScroll },
        { L"X:",       IDC_PROP_X,      Win32::Styles::EditAutoHScroll },
        { L"Y:",       IDC_PROP_Y,      Win32::Styles::EditAutoHScroll },
        { L"Width:",   IDC_PROP_W,      Win32::Styles::EditAutoHScroll },
        { L"Height:",  IDC_PROP_H,      Win32::Styles::EditAutoHScroll },
        { L"onClick:",  IDC_PROP_ONCLICK,     Win32::Styles::EditAutoHScroll },
        { L"onChange:",  IDC_PROP_ONCHANGE,    Win32::Styles::EditAutoHScroll },
        { L"onDblClk:", IDC_PROP_ONDBLCLICK,  Win32::Styles::EditAutoHScroll },
        { L"onSelChg:", IDC_PROP_ONSELCHANGE, Win32::Styles::EditAutoHScroll },
        { L"onFocus:", IDC_PROP_ONFOCUS,     Win32::Styles::EditAutoHScroll },
        { L"onBlur:",  IDC_PROP_ONBLUR,      Win32::Styles::EditAutoHScroll },
        { L"onCheck:", IDC_PROP_ONCHECK,     Win32::Styles::EditAutoHScroll },
        { L"TabIdx:", IDC_PROP_TABINDEX,    Win32::Styles::EditAutoHScroll },
    };

    int y = 30;
    for (auto& row : ctrlRows)
    {
        auto lbl = Win32::CreateWindowExW(0, L"STATIC", row.label,
            Win32::Styles::Child | Win32::Styles::StaticRight,
            5, y + 2, 55, 18, parent,
            reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId + IDL_OFFSET)),
            hInst, nullptr);
        Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

        auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
            Win32::Styles::Child | Win32::Styles::TabStop | row.extraStyle,
            65, y, 150, 22, parent,
            reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId)),
            hInst, nullptr);
        Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
        Win32::EnableWindow(edit, false);

        y += 26;
    }

    // Text alignment dropdown (ComboBox instead of EDIT).
    {
        auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Align:",
            Win32::Styles::Child | Win32::Styles::StaticRight,
            5, y + 2, 55, 18, parent,
            reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_TEXTALIGN + IDL_OFFSET)),
            hInst, nullptr);
        Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

        auto combo = Win32::CreateWindowExW(0, Win32::Controls::ComboBox, nullptr,
            Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::ComboBoxDropDownList,
            65, y, 150, 120, parent,
            reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_TEXTALIGN)),
            hInst, nullptr);
        Win32::SendMessageW(combo, Win32::Messages::SetFont, font, true);
        Win32::SendMessageW(combo, Win32::ComboBox::AddString, 0,
            reinterpret_cast<Win32::LPARAM>(L"Left"));
        Win32::SendMessageW(combo, Win32::ComboBox::AddString, 0,
            reinterpret_cast<Win32::LPARAM>(L"Center"));
        Win32::SendMessageW(combo, Win32::ComboBox::AddString, 0,
            reinterpret_cast<Win32::LPARAM>(L"Right"));
        Win32::SendMessageW(combo, Win32::ComboBox::SetCurSel, 0, 0);
    }

    // Form property rows (visible when no control is selected).
    PropRow formRows[] = {
        { L"Title:",   IDC_PROP_FORM_TITLE,  Win32::Styles::EditAutoHScroll },
        { L"Width:",   IDC_PROP_FORM_WIDTH,  Win32::Styles::EditAutoHScroll },
        { L"Height:",  IDC_PROP_FORM_HEIGHT, Win32::Styles::EditAutoHScroll },
        { L"BgColor:", IDC_PROP_FORM_BGCOLOR, Win32::Styles::EditAutoHScroll },
    };

    y = 30;
    for (auto& row : formRows)
    {
        auto lbl = Win32::CreateWindowExW(0, L"STATIC", row.label,
            Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticRight,
            5, y + 2, 55, 18, parent,
            reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId + IDL_OFFSET)),
            hInst, nullptr);
        Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

        int editW = (row.editId == IDC_PROP_FORM_BGCOLOR) ? 105 : 150;
        auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
            Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::TabStop | row.extraStyle,
            65, y, editW, 22, parent,
            reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId)),
            hInst, nullptr);
        Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);

        y += 26;
    }

    // Color picker button next to BgColor.
    auto bgBtn = Win32::CreateWindowExW(0, L"BUTTON", L"...",
        Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
        175, 30 + 3 * 26, 40, 22, parent,
        reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_BGCOLOR_BTN)),
        hInst, nullptr);
    Win32::SendMessageW(bgBtn, Win32::Messages::SetFont, font, true);
}

constexpr int PROP_CONTENT_CTRL = 30 + 16 * 26 + 10;  // control properties: 456px
constexpr int PROP_CONTENT_FORM = 30 + 4 * 26 + 10;   // form properties: 144px
constexpr int SCROLL_LINE = 26;                         // one row height

void UpdateScrollRange(DesignState& state)
{
    auto panel = state.propertyHwnd;
    int sel = SingleSelection(state);
    bool hasSel = sel >= 0 && sel < static_cast<int>(state.entries.size());
    int contentHeight = hasSel ? PROP_CONTENT_CTRL : PROP_CONTENT_FORM;

    Win32::RECT rc;
    Win32::GetClientRect(panel, &rc);

    Win32::SCROLLINFO si = {
        .cbSize = sizeof(Win32::SCROLLINFO),
        .fMask = Win32::ScrollInfo::Range | Win32::ScrollInfo::Page,
        .nMin = 0,
        .nMax = contentHeight,
        .nPage = static_cast<Win32::UINT>(rc.bottom),
    };
    Win32::SetScrollInfo(panel, Win32::ScrollBar::Vert, &si, true);
}

void ScrollPropertyPanel(DesignState& state, int newPos)
{
    auto panel = state.propertyHwnd;

    // Clamp.
    Win32::SCROLLINFO si = { .cbSize = sizeof(Win32::SCROLLINFO), .fMask = Win32::ScrollInfo::All };
    Win32::GetScrollInfo(panel, Win32::ScrollBar::Vert, &si);
    int maxPos = si.nMax - static_cast<int>(si.nPage);
    if (maxPos < 0) maxPos = 0;
    if (newPos < 0) newPos = 0;
    if (newPos > maxPos) newPos = maxPos;

    int delta = state.propertyScrollY - newPos;
    if (delta == 0) return;

    state.propertyScrollY = newPos;
    si.fMask = Win32::ScrollInfo::Pos;
    si.nPos = newPos;
    Win32::SetScrollInfo(panel, Win32::ScrollBar::Vert, &si, true);
    Win32::ScrollWindowEx(panel, 0, delta, nullptr, nullptr, nullptr, nullptr,
        Win32::ScrollWindow::ScrollChildren | Win32::ScrollWindow::Invalidate |
        Win32::ScrollWindow::Erase);
}

void ResetPropertyScroll(DesignState& state)
{
    if (state.propertyScrollY != 0)
        ScrollPropertyPanel(state, 0);
}

Win32::COLORREF g_customColors[16] = {};

export auto PropertyPanelProc(Win32::HWND hwnd, Win32::UINT msg,
    Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
{
    auto* state = reinterpret_cast<DesignState*>(
        Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

    switch (msg)
    {
    case Win32::Messages::Size:
    {
        if (state) UpdateScrollRange(*state);
        break;
    }

    case Win32::Messages::VScroll:
    {
        if (!state) break;
        Win32::SCROLLINFO si = { .cbSize = sizeof(Win32::SCROLLINFO), .fMask = Win32::ScrollInfo::All };
        Win32::GetScrollInfo(hwnd, Win32::ScrollBar::Vert, &si);
        int pos = state->propertyScrollY;

        switch (Win32::GetLowWord(wParam))
        {
        case Win32::ScrollBar::LineUp:        pos -= SCROLL_LINE; break;
        case Win32::ScrollBar::LineDown:      pos += SCROLL_LINE; break;
        case Win32::ScrollBar::PageUp:        pos -= static_cast<int>(si.nPage); break;
        case Win32::ScrollBar::PageDown:      pos += static_cast<int>(si.nPage); break;
        case Win32::ScrollBar::ThumbTrack:    pos = si.nTrackPos; break;
        case Win32::ScrollBar::ThumbPosition: pos = si.nTrackPos; break;
        case Win32::ScrollBar::Top:           pos = si.nMin; break;
        case Win32::ScrollBar::Bottom:        pos = si.nMax; break;
        }
        ScrollPropertyPanel(*state, pos);
        return 0;
    }

    case Win32::Messages::MouseWheel:
    {
        if (!state) break;
        int delta = Win32::GetWheelDelta(wParam);
        int lines = delta / 120;
        ScrollPropertyPanel(*state, state->propertyScrollY - lines * SCROLL_LINE);
        return 0;
    }

    case Win32::Messages::Command:
    {
        if (!state || state->updatingProperties) break;

        auto code = Win32::GetHighWord(wParam);
        auto id   = Win32::GetLowWord(wParam);

        // Color picker button.
        if (id == IDC_PROP_FORM_BGCOLOR_BTN && code == Win32::Notifications::ButtonClicked)
        {
            Win32::COLORREF initial = (state->form.backgroundColor != -1)
                ? static_cast<Win32::COLORREF>(state->form.backgroundColor)
                : Win32::GetSysColor(Win32::ColorBtnFace);

            Win32::CHOOSECOLORW cc = {
                .lStructSize = sizeof(Win32::CHOOSECOLORW),
                .hwndOwner = state->surfaceHwnd,
                .rgbResult = initial,
                .lpCustColors = g_customColors,
                .Flags = Win32::ColorDialog::FullOpen | Win32::ColorDialog::RgbInit,
            };

            if (Win32::ChooseColorW(&cc))
            {
                PushUndo(*state);
                state->form.backgroundColor = static_cast<int>(cc.rgbResult);
                UpdatePropertyPanel(*state);
                Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
                MarkDirty(*state);
            }
            return 0;
        }

        // Property edits (apply on focus loss).
        if (code == Win32::Notifications::EditKillFocus)
        {
            if (id >= IDC_PROP_TYPE && id <= IDC_PROP_TABINDEX)
                ApplyPropertyChange(*state, id);
            else if (id >= IDC_PROP_FORM_TITLE && id <= IDC_PROP_FORM_BGCOLOR)
                ApplyFormPropertyChange(*state, id);
            return 0;
        }

        // Alignment dropdown change.
        if (id == IDC_PROP_TEXTALIGN && code == Win32::Notifications::ComboBoxSelEndOk)
        {
            ApplyPropertyChange(*state, IDC_PROP_TEXTALIGN);
            return 0;
        }
        break;
    }
    }

    return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
}

}
