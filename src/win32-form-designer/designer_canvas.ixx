module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>

export module designer:canvas;
import std;
import formbuilder;
import :state;
import :helpers;
import :properties;

namespace Designer
{

export void PopulateControls(DesignState& state)
{
    for (auto& control : state.form.controls)
    {
        auto* className = FormDesigner::ClassNameFor(control.type);
        if (!className)
            continue;

        auto style = static_cast<DWORD>(
            WS_CHILD | WS_VISIBLE |
            FormDesigner::ImpliedStyleFor(control.type) |
            control.style);

        auto hwnd = CreateWindowExW(
            control.exStyle,
            className,
            control.text.c_str(),
            style,
            control.rect.x, control.rect.y,
            control.rect.width, control.rect.height,
            state.canvasHwnd,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(control.id)),
            state.hInstance,
            nullptr);

        if (!hwnd)
            continue;

        SendMessageW(hwnd, WM_SETFONT,
            reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);

        SetWindowSubclass(hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
        state.entries.push_back({ &control, hwnd });
    }
}

export void RebuildControls(DesignState& state)
{
    for (auto& entry : state.entries)
        DestroyWindow(entry.hwnd);
    state.entries.clear();
    state.selectedIndex = -1;
    PopulateControls(state);
    InvalidateRect(state.canvasHwnd, nullptr, TRUE);
    UpdatePropertyPanel(state);
}

void PlaceControl(DesignState& state, int x, int y)
{
    auto newId = NextControlId(state);
    auto& ctrl = state.form.controls.emplace_back();

    for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
        state.entries[i].control = &state.form.controls[i];

    ctrl.type = state.placementType;
    ctrl.id = newId;
    ctrl.rect = { x, y, 100, 25 };

    switch (ctrl.type)
    {
    case FormDesigner::ControlType::Button:   ctrl.text = L"Button"; break;
    case FormDesigner::ControlType::CheckBox: ctrl.text = L"CheckBox"; break;
    case FormDesigner::ControlType::Label:    ctrl.text = L"Label"; break;
    case FormDesigner::ControlType::TextBox:  ctrl.text = L"TextBox"; break;
    case FormDesigner::ControlType::GroupBox: ctrl.text = L"GroupBox"; ctrl.rect.height = 100; break;
    case FormDesigner::ControlType::ListBox:  ctrl.rect.height = 80; break;
    case FormDesigner::ControlType::ComboBox: break;
    default: ctrl.text = L"Control"; break;
    }

    auto* className = FormDesigner::ClassNameFor(ctrl.type);
    auto style = static_cast<DWORD>(
        WS_CHILD | WS_VISIBLE |
        FormDesigner::ImpliedStyleFor(ctrl.type) |
        ctrl.style);

    auto hwnd = CreateWindowExW(
        ctrl.exStyle,
        className,
        ctrl.text.c_str(),
        style,
        ctrl.rect.x, ctrl.rect.y,
        ctrl.rect.width, ctrl.rect.height,
        state.canvasHwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ctrl.id)),
        state.hInstance,
        nullptr);

    if (hwnd)
    {
        SendMessageW(hwnd, WM_SETFONT,
            reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
        SetWindowSubclass(hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
        state.entries.push_back({ &ctrl, hwnd });
        state.selectedIndex = static_cast<int>(state.entries.size()) - 1;
    }

    state.placementMode = false;
    SendMessageW(state.toolboxHwnd, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
    MarkDirty(state);
    InvalidateRect(state.canvasHwnd, nullptr, TRUE);
    UpdatePropertyPanel(state);
}

export void CancelPlacement(DesignState& state)
{
    if (state.placementMode)
    {
        state.placementMode = false;
        SendMessageW(state.toolboxHwnd, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
    }
}

void DeleteSelectedControl(DesignState& state)
{
    if (state.selectedIndex < 0 ||
        state.selectedIndex >= static_cast<int>(state.entries.size()))
        return;

    DestroyWindow(state.entries[state.selectedIndex].hwnd);
    state.form.controls.erase(
        state.form.controls.begin() + state.selectedIndex);
    state.entries.erase(
        state.entries.begin() + state.selectedIndex);

    for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
        state.entries[i].control = &state.form.controls[i];

    state.selectedIndex = -1;
    MarkDirty(state);
    InvalidateRect(state.canvasHwnd, nullptr, TRUE);
    UpdatePropertyPanel(state);
}

export LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto* state = reinterpret_cast<DesignState*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_ERASEBKGND:
    {
        if (state && state->form.backgroundColor != -1)
        {
            auto hdc = reinterpret_cast<HDC>(wParam);
            RECT rc;
            GetClientRect(hwnd, &rc);
            auto brush = CreateSolidBrush(
                static_cast<COLORREF>(state->form.backgroundColor));
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);
            return 1;
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);

        if (state)
        {
            auto hdc = GetDC(hwnd);
            DrawSelection(*state, hdc);
            ReleaseDC(hwnd, hdc);
        }
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        if (!state) break;
        SetFocus(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        if (state->placementMode)
        {
            PlaceControl(*state, x, y);
            return 0;
        }

        int handle = HitTestHandle(*state, x, y);
        if (handle >= 0)
        {
            auto& r = state->entries[state->selectedIndex].control->rect;
            state->dragMode = DragMode::Resize;
            state->activeHandle = handle;
            state->dragStart = { x, y };
            state->controlStart = { r.x, r.y };
            state->controlStartSize = { r.width, r.height };
            SetCapture(hwnd);
            return 0;
        }

        int hit = HitTest(*state, x, y);
        state->selectedIndex = hit;
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdatePropertyPanel(*state);

        if (hit >= 0)
        {
            auto& r = state->entries[hit].control->rect;
            state->dragMode = DragMode::Move;
            state->activeHandle = -1;
            state->dragStart = { x, y };
            state->controlStart = { r.x, r.y };
            SetCapture(hwnd);
        }
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        if (!state) break;
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        if (state->dragMode == DragMode::Move)
        {
            auto& entry = state->entries[state->selectedIndex];
            entry.control->rect.x = state->controlStart.x + (x - state->dragStart.x);
            entry.control->rect.y = state->controlStart.y + (y - state->dragStart.y);

            MoveWindow(entry.hwnd,
                entry.control->rect.x, entry.control->rect.y,
                entry.control->rect.width, entry.control->rect.height,
                TRUE);
            InvalidateRect(hwnd, nullptr, TRUE);
            MarkDirty(*state);
            UpdatePropertyPanel(*state);
            return 0;
        }

        if (state->dragMode == DragMode::Resize)
        {
            auto& entry = state->entries[state->selectedIndex];
            int dx = x - state->dragStart.x;
            int dy = y - state->dragStart.y;

            ApplyResize(entry.control->rect, state->activeHandle, dx, dy,
                state->controlStart, state->controlStartSize);

            MoveWindow(entry.hwnd,
                entry.control->rect.x, entry.control->rect.y,
                entry.control->rect.width, entry.control->rect.height,
                TRUE);
            InvalidateRect(hwnd, nullptr, TRUE);
            MarkDirty(*state);
            UpdatePropertyPanel(*state);
            return 0;
        }

        if (state->placementMode)
        {
            SetCursor(LoadCursorW(nullptr, IDC_CROSS));
        }
        else
        {
            int handle = HitTestHandle(*state, x, y);
            if (handle >= 0)
                SetCursor(LoadCursorW(nullptr, CursorForHandle(handle)));
            else
                SetCursor(LoadCursorW(nullptr, IDC_ARROW));
        }
        break;
    }

    case WM_LBUTTONUP:
    {
        if (!state || state->dragMode == DragMode::None) break;
        state->dragMode = DragMode::None;
        state->activeHandle = -1;
        ReleaseCapture();
        return 0;
    }

    case WM_SETCURSOR:
    {
        if (state && LOWORD(lParam) == HTCLIENT)
            return TRUE;
        break;
    }

    case WM_KEYDOWN:
    {
        if (!state) break;
        if (wParam == VK_DELETE)
        {
            DeleteSelectedControl(*state);
            return 0;
        }
        break;
    }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

}
