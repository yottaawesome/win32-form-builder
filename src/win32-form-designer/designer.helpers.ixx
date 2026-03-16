export module designer:helpers;
import std;
import formbuilder;
import :win32;
import :state;

namespace Designer
{

export auto ControlSubclassProc(
    Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam,
    Win32::UINT_PTR, Win32::DWORD_PTR) -> Win32::LRESULT
{
    if (msg == Win32::Messages::NcHitTest)
        return Win32::HitTestValues::Transparent;
    return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
}

// Destroys and recreates a single control's HWND (e.g. after style change).
export void RebuildSingleControl(DesignState& state, ControlEntry& entry)
{
    Win32::DestroyWindow(entry.hwnd);

    auto& ctrl = *entry.control;
    auto* className = FormDesigner::ClassNameFor(ctrl.type);
    if (!className) return;

    auto style = Win32::DWORD{
        Win32::Styles::Child | Win32::Styles::Visible |
        FormDesigner::ImpliedStyleFor(ctrl.type) |
        FormDesigner::AlignmentStyleFor(ctrl.type, ctrl.textAlign) |
        ctrl.style};

    entry.hwnd = Win32::CreateWindowExW(
        ctrl.exStyle, className, ctrl.text.c_str(), style,
        ctrl.rect.x, ctrl.rect.y, ctrl.rect.width, ctrl.rect.height,
        state.canvasHwnd,
        reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(ctrl.id)),
        state.hInstance, nullptr);

    if (entry.hwnd)
    {
        Win32::SendMessageW(entry.hwnd, Win32::Messages::SetFont,
            reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);
        Win32::SetWindowSubclass(entry.hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
    }
    Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
}

export auto IsSelected(const DesignState& state, int index) -> bool
{
    return state.selection.contains(index);
}

export auto SingleSelection(const DesignState& state) -> int
{
    if (state.selection.size() == 1)
        return *state.selection.begin();
    return -1;
}

export auto HitTest(const DesignState& state, int x, int y) -> int
{
    for (int i = static_cast<int>(state.entries.size()) - 1; i >= 0; --i)
    {
        auto& r = state.entries[i].control->rect;
        if (x >= r.x && x < r.x + r.width &&
            y >= r.y && y < r.y + r.height)
            return i;
    }
    return -1;
}

export void GetHandleAnchors(const FormDesigner::Rect& r, Win32::POINT out[8])
{
    int cx = r.x + r.width / 2;
    int cy = r.y + r.height / 2;
    int rx = r.x + r.width;
    int by = r.y + r.height;

    out[0] = { r.x - HANDLE_HALF, r.y - HANDLE_HALF };
    out[1] = { cx  - HANDLE_HALF, r.y - HANDLE_HALF };
    out[2] = { rx  - HANDLE_HALF, r.y - HANDLE_HALF };
    out[3] = { r.x - HANDLE_HALF, cy  - HANDLE_HALF };
    out[4] = { rx  - HANDLE_HALF, cy  - HANDLE_HALF };
    out[5] = { r.x - HANDLE_HALF, by  - HANDLE_HALF };
    out[6] = { cx  - HANDLE_HALF, by  - HANDLE_HALF };
    out[7] = { rx  - HANDLE_HALF, by  - HANDLE_HALF };
}

export auto HitTestHandle(const DesignState& state, int x, int y) -> int
{
    int sel = SingleSelection(state);
    if (sel < 0 || sel >= static_cast<int>(state.entries.size()))
        return -1;

    Win32::POINT anchors[8];
    GetHandleAnchors(state.entries[sel].control->rect, anchors);

    for (int i = 0; i < 8; ++i)
    {
        if (x >= anchors[i].x && x < anchors[i].x + HANDLE_SIZE &&
            y >= anchors[i].y && y < anchors[i].y + HANDLE_SIZE)
            return i;
    }
    return -1;
}

export auto CursorForHandle(int handle) -> Win32::LPCWSTR
{
    switch (handle)
    {
    case 0: case 7: return Win32::Cursors::SizeNWSE;
    case 2: case 5: return Win32::Cursors::SizeNESW;
    case 1: case 6: return Win32::Cursors::SizeNS;
    case 3: case 4: return Win32::Cursors::SizeWE;
    default:         return Win32::Cursors::Arrow;
    }
}

export void ApplyResize(FormDesigner::Rect& r, int handle, int dx, int dy,
    const Win32::POINT& startPos, const Win32::SIZE& startSize)
{
    bool moveLeft   = (handle == 0 || handle == 3 || handle == 5);
    bool moveTop    = (handle == 0 || handle == 1 || handle == 2);
    bool moveRight  = (handle == 2 || handle == 4 || handle == 7);
    bool moveBottom = (handle == 5 || handle == 6 || handle == 7);

    int newX = startPos.x;
    int newY = startPos.y;
    int newW = startSize.cx;
    int newH = startSize.cy;

    if (moveLeft)   { newX += dx; newW -= dx; }
    if (moveTop)    { newY += dy; newH -= dy; }
    if (moveRight)  { newW += dx; }
    if (moveBottom) { newH += dy; }

    if (newW < MIN_CONTROL_SIZE)
    {
        if (moveLeft) newX -= (MIN_CONTROL_SIZE - newW);
        newW = MIN_CONTROL_SIZE;
    }
    if (newH < MIN_CONTROL_SIZE)
    {
        if (moveTop) newY -= (MIN_CONTROL_SIZE - newH);
        newH = MIN_CONTROL_SIZE;
    }

    r.x = newX;
    r.y = newY;
    r.width = newW;
    r.height = newH;
}

export void DrawSelection(const DesignState& state, Win32::HDC hdc)
{
    if (state.selection.empty()) return;

    auto accent = Win32::CreateSolidBrush(Win32::MakeRgb(0, 120, 215));
    auto locked = Win32::CreateSolidBrush(Win32::MakeRgb(128, 128, 128));

    for (int idx : state.selection)
    {
        if (idx < 0 || idx >= static_cast<int>(state.entries.size()))
            continue;

        auto& ctrl = *state.entries[idx].control;
        auto& r = ctrl.rect;
        auto brush = ctrl.locked ? locked : accent;

        Win32::RECT sides[] = {
            { r.x - 2, r.y - 2,          r.x + r.width + 2, r.y },
            { r.x - 2, r.y + r.height,   r.x + r.width + 2, r.y + r.height + 2 },
            { r.x - 2, r.y,              r.x,                r.y + r.height },
            { r.x + r.width, r.y,        r.x + r.width + 2,  r.y + r.height },
        };
        for (auto& s : sides)
            Win32::FillRect(hdc, &s, brush);
    }

    // Resize handles only when exactly one unlocked control is selected.
    int sel = SingleSelection(state);
    if (sel >= 0 && sel < static_cast<int>(state.entries.size()) &&
        !state.entries[sel].control->locked)
    {
        auto& r = state.entries[sel].control->rect;
        Win32::POINT anchors[8];
        GetHandleAnchors(r, anchors);

        auto white = Win32::CreateSolidBrush(Win32::MakeRgb(255, 255, 255));
        for (auto& a : anchors)
        {
            Win32::RECT outer = { a.x, a.y, a.x + HANDLE_SIZE, a.y + HANDLE_SIZE };
            Win32::RECT inner = { a.x + 1, a.y + 1, a.x + HANDLE_SIZE - 1, a.y + HANDLE_SIZE - 1 };
            Win32::FillRect(hdc, &outer, accent);
            Win32::FillRect(hdc, &inner, white);
        }
        Win32::DeleteObject(white);
    }

    Win32::DeleteObject(locked);
    Win32::DeleteObject(accent);
}

export void UpdateTitle(DesignState& state)
{
    auto name = state.currentFile.empty()
        ? std::wstring(L"Untitled")
        : state.currentFile.filename().wstring();
    auto title = std::wstring(L"Form Designer - ");
    if (state.dirty)
        title += L"*";
    title += name;
    Win32::SetWindowTextW(state.surfaceHwnd, title.c_str());
}

export void MarkDirty(DesignState& state)
{
    if (!state.dirty)
    {
        state.dirty = true;
        UpdateTitle(state);
    }
}

export auto ControlTypeDisplayName(FormDesigner::ControlType type) -> const wchar_t*
{
    switch (type)
    {
    case FormDesigner::ControlType::Button:      return L"Button";
    case FormDesigner::ControlType::CheckBox:    return L"CheckBox";
    case FormDesigner::ControlType::RadioButton: return L"RadioButton";
    case FormDesigner::ControlType::Label:       return L"Label";
    case FormDesigner::ControlType::TextBox:     return L"TextBox";
    case FormDesigner::ControlType::GroupBox:     return L"GroupBox";
    case FormDesigner::ControlType::ListBox:     return L"ListBox";
    case FormDesigner::ControlType::ComboBox:    return L"ComboBox";
    case FormDesigner::ControlType::ProgressBar:     return L"ProgressBar";
    case FormDesigner::ControlType::TrackBar:        return L"TrackBar";
    case FormDesigner::ControlType::DateTimePicker:  return L"DateTimePicker";
    case FormDesigner::ControlType::TabControl:      return L"TabControl";
    case FormDesigner::ControlType::ListView:        return L"ListView";
    case FormDesigner::ControlType::TreeView:        return L"TreeView";
    case FormDesigner::ControlType::UpDown:          return L"UpDown";
    case FormDesigner::ControlType::RichEdit:        return L"RichEdit";
    default:                                  return L"Window";
    }
}

export auto ColorRefToHex(int colorRef) -> std::wstring
{
    if (colorRef == -1) return {};
    auto cr = static_cast<unsigned int>(colorRef);
    auto r = cr & 0xFF;
    auto g = (cr >> 8) & 0xFF;
    auto b = (cr >> 16) & 0xFF;
    auto s = std::format("#{:02X}{:02X}{:02X}", r, g, b);
    return std::wstring(s.begin(), s.end());
}

export auto HexToColorRef(const std::wstring& hex) -> int
{
    if (hex.size() != 7 || hex[0] != L'#') return -1;
    auto s = std::string(hex.begin(), hex.end());
    unsigned int r = std::stoul(s.substr(1, 2), nullptr, 16);
    unsigned int g = std::stoul(s.substr(3, 2), nullptr, 16);
    unsigned int b = std::stoul(s.substr(5, 2), nullptr, 16);
    return static_cast<int>(r | (g << 8) | (b << 16));
}

export auto NextControlId(const DesignState& state) -> int
{
    int maxId = 0;
    for (auto& c : state.form.controls)
        if (c.id > maxId) maxId = c.id;
    return maxId + 1;
}

export void PushUndo(DesignState& state)
{
    state.undoStack.push_back(state.form);
    state.redoStack.clear();
}

export int SnapValue(int value, int gridSize)
{
    return ((value + gridSize / 2) / gridSize) * gridSize;
}

export void SnapRectToGrid(FormDesigner::Rect& rect, int gridSize)
{
    rect.x = SnapValue(rect.x, gridSize);
    rect.y = SnapValue(rect.y, gridSize);
}

// Compares edges of the moving control against all other controls.
// Snaps the rect in-place when within threshold and populates state.guides.
export void FindAlignGuides(DesignState& state, FormDesigner::Rect& rect)
{
    state.guides.clear();

    // 5 reference values for the moving control: left, center-x, right, top, center-y, bottom
    auto edges = [](const FormDesigner::Rect& r)
    {
        return std::array<int, 6>{
            r.x,
            r.x + r.width / 2,
            r.x + r.width,
            r.y,
            r.y + r.height / 2,
            r.y + r.height,
        };
    };

    bool snappedX = false;
    bool snappedY = false;

    for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
    {
        if (state.selection.contains(i)) continue;
        auto& other = state.entries[i].control->rect;
        auto otherEdges = edges(other);

        auto movingEdges = edges(rect);

        // Check vertical alignment (shared X positions)
        if (!snappedX)
        {
            for (int m = 0; m < 3; ++m)
            {
                for (int o = 0; o < 3; ++o)
                {
                    int diff = movingEdges[m] - otherEdges[o];
                    if (std::abs(diff) <= SNAP_THRESHOLD)
                    {
                        // Snap: adjust rect.x so moving edge m aligns with other edge o
                        rect.x -= diff;
                        state.guides.push_back({ false, otherEdges[o] });
                        snappedX = true;
                        break;
                    }
                }
                if (snappedX) break;
            }
        }

        // Check horizontal alignment (shared Y positions)
        if (!snappedY)
        {
            for (int m = 3; m < 6; ++m)
            {
                for (int o = 3; o < 6; ++o)
                {
                    int diff = movingEdges[m] - otherEdges[o];
                    if (std::abs(diff) <= SNAP_THRESHOLD)
                    {
                        rect.y -= diff;
                        state.guides.push_back({ true, otherEdges[o] });
                        snappedY = true;
                        break;
                    }
                }
                if (snappedY) break;
            }
        }

        if (snappedX && snappedY) break;
    }
}

export void DrawAlignGuides(const DesignState& state, Win32::HDC hdc)
{
    if (state.guides.empty()) return;

    Win32::RECT rc;
    Win32::GetClientRect(state.canvasHwnd, &rc);

    auto pen = Win32::CreatePen(Win32::PenStyles::Dot, 0, Win32::MakeRgb(255, 0, 128));
    auto oldPen = Win32::SelectObject(hdc, pen);
    auto oldMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);

    for (auto& guide : state.guides)
    {
        if (guide.horizontal)
        {
            Win32::MoveToEx(hdc, rc.left, guide.position, nullptr);
            Win32::LineTo(hdc, rc.right, guide.position);
        }
        else
        {
            Win32::MoveToEx(hdc, guide.position, rc.top, nullptr);
            Win32::LineTo(hdc, guide.position, rc.bottom);
        }
    }

    Win32::SetBkMode(hdc, oldMode);
    Win32::SelectObject(hdc, oldPen);
    Win32::DeleteObject(pen);
}

}
