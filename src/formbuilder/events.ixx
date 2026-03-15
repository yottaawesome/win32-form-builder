export module formbuilder:events;
import std;
import :win32;

export namespace FormDesigner
{
struct ClickEvent
{
int controlId;
Win32::HWND controlHwnd;
Win32::HWND formHwnd;
};

struct ChangeEvent
{
int controlId;
Win32::HWND controlHwnd;
Win32::HWND formHwnd;
};

struct DoubleClickEvent
{
int controlId;
Win32::HWND controlHwnd;
Win32::HWND formHwnd;
};

struct SelectionChangeEvent
{
int controlId;
Win32::HWND controlHwnd;
Win32::HWND formHwnd;
};

struct EventMap
{
void onClick(int controlId, std::function<void(const ClickEvent&)> handler)
{
m_clickHandlers[controlId] = std::move(handler);
}

void onChange(int controlId, std::function<void(const ChangeEvent&)> handler)
{
m_changeHandlers[controlId] = std::move(handler);
}

void onDoubleClick(int controlId, std::function<void(const DoubleClickEvent&)> handler)
{
m_doubleClickHandlers[controlId] = std::move(handler);
}

void onSelectionChange(int controlId, std::function<void(const SelectionChangeEvent&)> handler)
{
m_selectionChangeHandlers[controlId] = std::move(handler);
}

auto findClickHandler(int controlId) const -> const std::function<void(const ClickEvent&)>*
{
if (auto it = m_clickHandlers.find(controlId); it != m_clickHandlers.end())
return &it->second;
return nullptr;
}

auto findChangeHandler(int controlId) const -> const std::function<void(const ChangeEvent&)>*
{
if (auto it = m_changeHandlers.find(controlId); it != m_changeHandlers.end())
return &it->second;
return nullptr;
}

auto findDoubleClickHandler(int controlId) const -> const std::function<void(const DoubleClickEvent&)>*
{
if (auto it = m_doubleClickHandlers.find(controlId); it != m_doubleClickHandlers.end())
return &it->second;
return nullptr;
}

auto findSelectionChangeHandler(int controlId) const -> const std::function<void(const SelectionChangeEvent&)>*
{
if (auto it = m_selectionChangeHandlers.find(controlId); it != m_selectionChangeHandlers.end())
return &it->second;
return nullptr;
}

private:
std::unordered_map<int, std::function<void(const ClickEvent&)>> m_clickHandlers;
std::unordered_map<int, std::function<void(const ChangeEvent&)>> m_changeHandlers;
std::unordered_map<int, std::function<void(const DoubleClickEvent&)>> m_doubleClickHandlers;
std::unordered_map<int, std::function<void(const SelectionChangeEvent&)>> m_selectionChangeHandlers;
};
}
