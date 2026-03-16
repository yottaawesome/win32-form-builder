export module designer:canvas;
import std;
import formbuilder;
import :win32;
import :state;
import :helpers;
import :properties;

namespace Designer
{

void RefreshZOrderPanel(DesignState& state);

export void PopulateControls(DesignState& state)
{
    for (auto& control : state.form.controls)
    {
        auto* className = FormDesigner::ClassNameFor(control.type);
        if (!className)
            continue;

        auto style = Win32::DWORD{
            Win32::Styles::Child | Win32::Styles::Visible |
            FormDesigner::ImpliedStyleFor(control.type) |
            control.style};

        auto hwnd = Win32::CreateWindowExW(
            control.exStyle,
            className,
            control.text.c_str(),
            style,
            control.rect.x, control.rect.y,
            control.rect.width, control.rect.height,
            state.canvasHwnd,
            reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(control.id)),
            state.hInstance,
            nullptr);

        if (!hwnd)
            continue;

        Win32::SendMessageW(hwnd, Win32::Messages::SetFont,
            reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);

        Win32::SetWindowSubclass(hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
        state.entries.push_back({ &control, hwnd });
    }
}

export void RebuildControls(DesignState& state)
{
    for (auto& entry : state.entries)
        Win32::DestroyWindow(entry.hwnd);
    state.entries.clear();
    state.selection.clear();
    PopulateControls(state);
    Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
    UpdatePropertyPanel(state);
    RefreshZOrderPanel(state);
}

void PlaceControl(DesignState& state, int x, int y)
{
    PushUndo(state);
    auto newId = NextControlId(state);
    auto& ctrl = state.form.controls.emplace_back();

    for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
        state.entries[i].control = &state.form.controls[i];

    ctrl.type = state.placementType;
    ctrl.id = newId;
    ctrl.rect = { x, y, 100, 25 };

    switch (ctrl.type)
    {
    case FormDesigner::ControlType::Button:      ctrl.text = L"Button"; break;
    case FormDesigner::ControlType::CheckBox:    ctrl.text = L"CheckBox"; break;
    case FormDesigner::ControlType::RadioButton: ctrl.text = L"RadioButton"; break;
    case FormDesigner::ControlType::Label:       ctrl.text = L"Label"; break;
    case FormDesigner::ControlType::TextBox:     ctrl.text = L"TextBox"; break;
    case FormDesigner::ControlType::GroupBox:    ctrl.text = L"GroupBox"; ctrl.rect.height = 100; break;
    case FormDesigner::ControlType::ListBox:     ctrl.rect.height = 80; break;
    case FormDesigner::ControlType::ComboBox:    break;
    case FormDesigner::ControlType::ProgressBar:     ctrl.rect.height = 20; break;
    case FormDesigner::ControlType::TrackBar:        ctrl.rect.height = 30; break;
    case FormDesigner::ControlType::DateTimePicker:  break;
    case FormDesigner::ControlType::TabControl:      ctrl.rect = { x, y, 200, 150 }; break;
    case FormDesigner::ControlType::ListView:        ctrl.rect = { x, y, 200, 120 }; break;
    case FormDesigner::ControlType::TreeView:        ctrl.rect = { x, y, 200, 150 }; break;
    case FormDesigner::ControlType::UpDown:          ctrl.rect = { x, y, 20, 25 }; break;
    case FormDesigner::ControlType::RichEdit:        ctrl.rect = { x, y, 200, 100 }; break;
    default: ctrl.text = L"Control"; break;
    }

    auto* className = FormDesigner::ClassNameFor(ctrl.type);
    auto style = Win32::DWORD{
        Win32::Styles::Child | Win32::Styles::Visible |
        FormDesigner::ImpliedStyleFor(ctrl.type) |
        ctrl.style};

    auto hwnd = Win32::CreateWindowExW(
        ctrl.exStyle,
        className,
        ctrl.text.c_str(),
        style,
        ctrl.rect.x, ctrl.rect.y,
        ctrl.rect.width, ctrl.rect.height,
        state.canvasHwnd,
        reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(ctrl.id)),
        state.hInstance,
        nullptr);

    if (hwnd)
    {
        Win32::SendMessageW(hwnd, Win32::Messages::SetFont,
            reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);
        Win32::SetWindowSubclass(hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
        state.entries.push_back({ &ctrl, hwnd });
        state.selection = { static_cast<int>(state.entries.size()) - 1 };
    }

    state.placementMode = false;
    Win32::SendMessageW(state.toolboxHwnd, Win32::ListBox::SetCurSel,
        static_cast<Win32::WPARAM>(-1), 0);
    MarkDirty(state);
    Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
    UpdatePropertyPanel(state);
    RefreshZOrderPanel(state);
}

export void CancelPlacement(DesignState& state)
{
    if (state.placementMode)
    {
        state.placementMode = false;
        Win32::SendMessageW(state.toolboxHwnd, Win32::ListBox::SetCurSel,
            static_cast<Win32::WPARAM>(-1), 0);
    }
}

export void Undo(DesignState& state)
{
    if (state.undoStack.empty()) return;
    state.redoStack.push_back(std::move(state.form));
    state.form = std::move(state.undoStack.back());
    state.undoStack.pop_back();
    state.selection.clear();
    RebuildControls(state);
    MarkDirty(state);
}

export void Redo(DesignState& state)
{
    if (state.redoStack.empty()) return;
    state.undoStack.push_back(std::move(state.form));
    state.form = std::move(state.redoStack.back());
    state.redoStack.pop_back();
    state.selection.clear();
    RebuildControls(state);
    MarkDirty(state);
}

export void DeleteSelectedControls(DesignState& state);

export void SelectAll(DesignState& state)
{
    for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
        state.selection.insert(i);
    Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
    UpdatePropertyPanel(state);
}

export void MoveControlInZOrder(DesignState& state, int from, int to)
{
    if (from == to) return;
    int count = static_cast<int>(state.form.controls.size());
    if (from < 0 || from >= count || to < 0 || to >= count) return;

    PushUndo(state);

    // Swap adjacent entries to move from→to, preserving relative order of others.
    int step = (to > from) ? 1 : -1;
    for (int i = from; i != to; i += step)
    {
        int next = i + step;
        std::swap(state.form.controls[i], state.form.controls[next]);
        std::swap(state.entries[i], state.entries[next]);
    }

    // Fix up control pointers.
    for (int i = 0; i < count; ++i)
        state.entries[i].control = &state.form.controls[i];

    // Update Win32 z-order to match vector order.
    Win32::HWND prevHwnd = Win32::HwndBottom;
    for (int i = 0; i < count; ++i)
    {
        Win32::SetWindowPos(state.entries[i].hwnd, prevHwnd, 0, 0, 0, 0,
            Win32::Swp::NoMove | Win32::Swp::NoSize | Win32::Swp::NoActivate);
        prevHwnd = state.entries[i].hwnd;
    }

    // Remap selection to follow the moved control.
    if (state.selection.contains(from) && !state.selection.contains(to))
    {
        state.selection.erase(from);
        state.selection.insert(to);
    }

    MarkDirty(state);
    Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
    UpdatePropertyPanel(state);
    RefreshZOrderPanel(state);
}

void RefreshZOrderPanel(DesignState& state)
{
    if (!state.zorderHwnd) return;
    auto list = Win32::GetDlgItem(state.zorderHwnd, IDC_ZORDER_LIST);
    if (!list) return;

    Win32::SendMessageW(list, Win32::ListBox::ResetContent, 0, 0);
    for (int i = 0; i < static_cast<int>(state.form.controls.size()); ++i)
    {
        auto& ctrl = state.form.controls[i];
        auto name = ControlTypeDisplayName(ctrl.type);
        auto textNarrow = std::string(ctrl.text.begin(), ctrl.text.end());
        auto label = std::format(L"{}: {} - {}", i, name,
            ctrl.text.empty() ? L"(no text)" : ctrl.text);
        Win32::SendMessageW(list, Win32::ListBox::AddString, 0,
            reinterpret_cast<Win32::LPARAM>(label.c_str()));
    }
}

export void CopySelected(DesignState& state)
{
    if (state.selection.empty()) return;
    state.clipboard.clear();
    for (int idx : state.selection)
    {
        if (idx >= 0 && idx < static_cast<int>(state.entries.size()))
            state.clipboard.push_back(*state.entries[idx].control);
    }
}

export void CutSelected(DesignState& state)
{
    CopySelected(state);
    if (!state.clipboard.empty())
        DeleteSelectedControls(state);
}

export void PasteControl(DesignState& state)
{
    if (state.clipboard.empty()) return;

    PushUndo(state);
    constexpr int PASTE_OFFSET = 20;

    state.selection.clear();

    for (auto& src : state.clipboard)
    {
        auto ctrl = src;
        ctrl.id = NextControlId(state);
        ctrl.rect.x += PASTE_OFFSET;
        ctrl.rect.y += PASTE_OFFSET;

        state.form.controls.push_back(std::move(ctrl));

        // Fix up existing entry pointers after push_back.
        for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
            state.entries[i].control = &state.form.controls[i];

        auto& placed = state.form.controls.back();
        auto* className = FormDesigner::ClassNameFor(placed.type);
        auto style = Win32::DWORD{
            Win32::Styles::Child | Win32::Styles::Visible |
            FormDesigner::ImpliedStyleFor(placed.type) |
            placed.style};

        auto hwnd = Win32::CreateWindowExW(
            placed.exStyle, className, placed.text.c_str(), style,
            placed.rect.x, placed.rect.y,
            placed.rect.width, placed.rect.height,
            state.canvasHwnd,
            reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(placed.id)),
            state.hInstance, nullptr);

        if (hwnd)
        {
            Win32::SendMessageW(hwnd, Win32::Messages::SetFont,
                reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);
            Win32::SetWindowSubclass(hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
            state.entries.push_back({ &placed, hwnd });
            state.selection.insert(static_cast<int>(state.entries.size()) - 1);
        }
    }

    MarkDirty(state);
    Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
    UpdatePropertyPanel(state);
}

export void DuplicateSelected(DesignState& state)
{
    CopySelected(state);
    PasteControl(state);
}

void DeleteSelectedControls(DesignState& state)
{
    if (state.selection.empty()) return;

    PushUndo(state);

    // Delete in reverse index order to keep earlier indices valid.
    std::vector<int> sorted(state.selection.begin(), state.selection.end());
    std::sort(sorted.rbegin(), sorted.rend());

    for (int idx : sorted)
    {
        if (idx < 0 || idx >= static_cast<int>(state.entries.size()))
            continue;
        Win32::DestroyWindow(state.entries[idx].hwnd);
        state.form.controls.erase(state.form.controls.begin() + idx);
        state.entries.erase(state.entries.begin() + idx);
    }

    for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
        state.entries[i].control = &state.form.controls[i];

    state.selection.clear();
    MarkDirty(state);
    Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
    UpdatePropertyPanel(state);
    RefreshZOrderPanel(state);
}

export auto CanvasProc(Win32::HWND hwnd, Win32::UINT msg,
    Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
{
    auto* state = reinterpret_cast<DesignState*>(
        Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

    switch (msg)
    {
    case Win32::Messages::EraseBkgnd:
    {
        if (state && state->form.backgroundColor != -1)
        {
            auto hdc = reinterpret_cast<Win32::HDC>(wParam);
            Win32::RECT rc;
            Win32::GetClientRect(hwnd, &rc);
            auto brush = Win32::CreateSolidBrush(
                static_cast<Win32::COLORREF>(state->form.backgroundColor));
            Win32::FillRect(hdc, &rc, brush);
            Win32::DeleteObject(brush);
            return 1;
        }
        break;
    }

    case Win32::Messages::Paint:
    {
        Win32::PAINTSTRUCT ps;
        Win32::BeginPaint(hwnd, &ps);
        Win32::EndPaint(hwnd, &ps);

        if (state)
        {
            auto hdc = Win32::GetDC(hwnd);
            DrawSelection(*state, hdc);
            DrawAlignGuides(*state, hdc);
            Win32::ReleaseDC(hwnd, hdc);
        }
        return 0;
    }

    case Win32::Messages::LButtonDown:
    {
        if (!state) break;
        Win32::SetFocus(hwnd);
        int x = Win32::GetXParam(lParam);
        int y = Win32::GetYParam(lParam);

        if (state->placementMode)
        {
            PlaceControl(*state, x, y);
            return 0;
        }

        bool ctrlHeld = (Win32::GetKeyState(Win32::Keys::Control) & 0x8000) != 0;

        // Resize handles: only when exactly one control is selected.
        int handle = HitTestHandle(*state, x, y);
        if (handle >= 0)
        {
            int sel = SingleSelection(*state);
            PushUndo(*state);
            auto& r = state->entries[sel].control->rect;
            state->dragMode = DragMode::Resize;
            state->activeHandle = handle;
            state->dragAnchor = sel;
            state->dragStart = { x, y };
            state->controlStart = { r.x, r.y };
            state->controlStartSize = { r.width, r.height };
            Win32::SetCapture(hwnd);
            return 0;
        }

        int hit = HitTest(*state, x, y);

        if (hit >= 0)
        {
            if (ctrlHeld)
            {
                // Ctrl+Click: toggle selection.
                if (IsSelected(*state, hit))
                    state->selection.erase(hit);
                else
                    state->selection.insert(hit);
            }
            else if (!IsSelected(*state, hit))
            {
                // Click on unselected without Ctrl: select only this one.
                state->selection = { hit };
            }
            // Click on already-selected without Ctrl: keep current selection (for drag).

            Win32::InvalidateRect(hwnd, nullptr, true);
            UpdatePropertyPanel(*state);

            // Start dragging all selected controls.
            if (IsSelected(*state, hit))
            {
                PushUndo(*state);
                state->dragMode = DragMode::Move;
                state->activeHandle = -1;
                state->dragAnchor = hit;
                state->dragStart = { x, y };
                state->dragOrigins.clear();
                for (int idx : state->selection)
                    state->dragOrigins[idx] = { state->entries[idx].control->rect.x,
                                                 state->entries[idx].control->rect.y };
                Win32::SetCapture(hwnd);
            }
        }
        else
        {
            // Click on empty space: deselect all.
            if (!ctrlHeld)
                state->selection.clear();
            Win32::InvalidateRect(hwnd, nullptr, true);
            UpdatePropertyPanel(*state);
        }
        return 0;
    }

    case Win32::Messages::MouseMove:
    {
        if (!state) break;
        int x = Win32::GetXParam(lParam);
        int y = Win32::GetYParam(lParam);

        if (state->dragMode == DragMode::Move)
        {
            int dx = x - state->dragStart.x;
            int dy = y - state->dragStart.y;

            // Move all selected controls by the same delta.
            for (int idx : state->selection)
            {
                auto it = state->dragOrigins.find(idx);
                if (it == state->dragOrigins.end()) continue;
                auto& entry = state->entries[idx];
                entry.control->rect.x = it->second.x + dx;
                entry.control->rect.y = it->second.y + dy;
            }

            // Snap guides based on the drag anchor control.
            if (state->dragAnchor >= 0 && state->dragAnchor < static_cast<int>(state->entries.size()))
            {
                auto& anchorRect = state->entries[state->dragAnchor].control->rect;
                auto prevX = anchorRect.x;
                auto prevY = anchorRect.y;
                FindAlignGuides(*state, anchorRect);
                int snapDx = anchorRect.x - prevX;
                int snapDy = anchorRect.y - prevY;
                // Apply snap adjustment to all other selected controls.
                if (snapDx != 0 || snapDy != 0)
                {
                    for (int idx : state->selection)
                    {
                        if (idx == state->dragAnchor) continue;
                        state->entries[idx].control->rect.x += snapDx;
                        state->entries[idx].control->rect.y += snapDy;
                    }
                }
            }

            for (int idx : state->selection)
            {
                auto& entry = state->entries[idx];
                Win32::MoveWindow(entry.hwnd,
                    entry.control->rect.x, entry.control->rect.y,
                    entry.control->rect.width, entry.control->rect.height,
                    true);
            }

            Win32::InvalidateRect(hwnd, nullptr, true);
            MarkDirty(*state);
            UpdatePropertyPanel(*state);
            return 0;
        }

        if (state->dragMode == DragMode::Resize)
        {
            int sel = SingleSelection(*state);
            if (sel < 0) break;
            auto& entry = state->entries[sel];
            int dx = x - state->dragStart.x;
            int dy = y - state->dragStart.y;

            ApplyResize(entry.control->rect, state->activeHandle, dx, dy,
                state->controlStart, state->controlStartSize);

            Win32::MoveWindow(entry.hwnd,
                entry.control->rect.x, entry.control->rect.y,
                entry.control->rect.width, entry.control->rect.height,
                true);
            Win32::InvalidateRect(hwnd, nullptr, true);
            MarkDirty(*state);
            UpdatePropertyPanel(*state);
            return 0;
        }

        if (state->placementMode)
        {
            Win32::SetCursor(Win32::LoadCursorW(nullptr, Win32::Cursors::Cross));
        }
        else
        {
            int handle = HitTestHandle(*state, x, y);
            if (handle >= 0)
                Win32::SetCursor(Win32::LoadCursorW(nullptr, CursorForHandle(handle)));
            else
                Win32::SetCursor(Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow));
        }
        break;
    }

    case Win32::Messages::LButtonUp:
    {
        if (!state || state->dragMode == DragMode::None) break;
        state->dragMode = DragMode::None;
        state->activeHandle = -1;
        state->guides.clear();
        Win32::ReleaseCapture();
        Win32::InvalidateRect(hwnd, nullptr, true);
        return 0;
    }

    case Win32::Messages::SetCursorMsg:
    {
        if (state && Win32::GetLowWord(static_cast<Win32::WPARAM>(lParam)) == Win32::HitTestValues::Client)
            return 1;
        break;
    }

    case Win32::Messages::KeyDown:
    {
        if (!state) break;
        if (wParam == Win32::Keys::Delete)
        {
            DeleteSelectedControls(*state);
            return 0;
        }
        if (wParam == 'A' && (Win32::GetKeyState(Win32::Keys::Control) & 0x8000))
        {
            SelectAll(*state);
            return 0;
        }
        break;
    }
    }

    return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
}

}
