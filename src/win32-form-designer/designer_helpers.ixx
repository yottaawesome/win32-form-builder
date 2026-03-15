module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>

export module designer:helpers;
import std;
import formbuilder;
import :state;

namespace Designer
{

export LRESULT CALLBACK ControlSubclassProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR, DWORD_PTR)
{
    if (msg == WM_NCHITTEST)
        return HTTRANSPARENT;
    return DefSubclassProc(hwnd, msg, wParam, lParam);
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

export void GetHandleAnchors(const FormDesigner::Rect& r, POINT out[8])
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
    if (state.selectedIndex < 0 ||
        state.selectedIndex >= static_cast<int>(state.entries.size()))
        return -1;

    POINT anchors[8];
    GetHandleAnchors(state.entries[state.selectedIndex].control->rect, anchors);

    for (int i = 0; i < 8; ++i)
    {
        if (x >= anchors[i].x && x < anchors[i].x + HANDLE_SIZE &&
            y >= anchors[i].y && y < anchors[i].y + HANDLE_SIZE)
            return i;
    }
    return -1;
}

export auto CursorForHandle(int handle) -> LPCWSTR
{
    switch (handle)
    {
    case 0: case 7: return IDC_SIZENWSE;
    case 2: case 5: return IDC_SIZENESW;
    case 1: case 6: return IDC_SIZENS;
    case 3: case 4: return IDC_SIZEWE;
    default:         return IDC_ARROW;
    }
}

export void ApplyResize(FormDesigner::Rect& r, int handle, int dx, int dy,
    const POINT& startPos, const SIZE& startSize)
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

export void DrawSelection(const DesignState& state, HDC hdc)
{
    if (state.selectedIndex < 0 ||
        state.selectedIndex >= static_cast<int>(state.entries.size()))
        return;

    auto& r = state.entries[state.selectedIndex].control->rect;

    auto accent = CreateSolidBrush(RGB(0, 120, 215));
    RECT sides[] = {
        { r.x - 2, r.y - 2,          r.x + r.width + 2, r.y },
        { r.x - 2, r.y + r.height,   r.x + r.width + 2, r.y + r.height + 2 },
        { r.x - 2, r.y,              r.x,                r.y + r.height },
        { r.x + r.width, r.y,        r.x + r.width + 2,  r.y + r.height },
    };
    for (auto& s : sides)
        FillRect(hdc, &s, accent);

    POINT anchors[8];
    GetHandleAnchors(r, anchors);

    auto white = CreateSolidBrush(RGB(255, 255, 255));
    for (auto& a : anchors)
    {
        RECT outer = { a.x, a.y, a.x + HANDLE_SIZE, a.y + HANDLE_SIZE };
        RECT inner = { a.x + 1, a.y + 1, a.x + HANDLE_SIZE - 1, a.y + HANDLE_SIZE - 1 };
        FillRect(hdc, &outer, accent);
        FillRect(hdc, &inner, white);
    }
    DeleteObject(white);
    DeleteObject(accent);
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
    SetWindowTextW(state.surfaceHwnd, title.c_str());
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
    case FormDesigner::ControlType::Button:   return L"Button";
    case FormDesigner::ControlType::CheckBox: return L"CheckBox";
    case FormDesigner::ControlType::Label:    return L"Label";
    case FormDesigner::ControlType::TextBox:  return L"TextBox";
    case FormDesigner::ControlType::GroupBox: return L"GroupBox";
    case FormDesigner::ControlType::ListBox:  return L"ListBox";
    case FormDesigner::ControlType::ComboBox: return L"ComboBox";
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

}
