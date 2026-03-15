export module designer;
import std;
import formbuilder;
export import :win32;
export import :state;
export import :helpers;
export import :properties;
export import :canvas;
export import :fileops;

namespace Designer
{

auto CreateMenuBar() -> Win32::HMENU
{
    auto menuBar = Win32::CreateMenu();
    auto fileMenu = Win32::CreatePopupMenu();

    Win32::AppendMenuW(fileMenu, Win32::Menu::String, IDM_FILE_NEW,     L"&New\tCtrl+N");
    Win32::AppendMenuW(fileMenu, Win32::Menu::String, IDM_FILE_OPEN,    L"&Open...\tCtrl+O");
    Win32::AppendMenuW(fileMenu, Win32::Menu::Separator, 0, nullptr);
    Win32::AppendMenuW(fileMenu, Win32::Menu::String, IDM_FILE_SAVE,    L"&Save\tCtrl+S");
    Win32::AppendMenuW(fileMenu, Win32::Menu::String, IDM_FILE_SAVE_AS, L"Save &As...\tCtrl+Shift+S");
    Win32::AppendMenuW(fileMenu, Win32::Menu::Separator, 0, nullptr);
    Win32::AppendMenuW(fileMenu, Win32::Menu::String, IDM_FILE_EXIT,    L"E&xit\tAlt+F4");

    Win32::AppendMenuW(menuBar, Win32::Menu::Popup,
        reinterpret_cast<Win32::UINT_PTR>(fileMenu), L"&File");
    return menuBar;
}

auto CreateAcceleratorTable() -> Win32::HACCEL
{
    Win32::ACCEL accels[] = {
        { Win32::Accel::Control | Win32::Accel::VirtKey, 'N', static_cast<Win32::WORD>(IDM_FILE_NEW) },
        { Win32::Accel::Control | Win32::Accel::VirtKey, 'O', static_cast<Win32::WORD>(IDM_FILE_OPEN) },
        { Win32::Accel::Control | Win32::Accel::VirtKey, 'S', static_cast<Win32::WORD>(IDM_FILE_SAVE) },
        { Win32::Accel::Control | Win32::Accel::Shift | Win32::Accel::VirtKey, 'S', static_cast<Win32::WORD>(IDM_FILE_SAVE_AS) },
        { Win32::Accel::VirtKey, static_cast<Win32::WORD>(Win32::Keys::Escape), static_cast<Win32::WORD>(IDM_CANCEL_PLACE) },
    };
    return Win32::CreateAcceleratorTableW(accels, static_cast<int>(std::size(accels)));
}

auto DesignSurfaceProc(Win32::HWND hwnd, Win32::UINT msg,
    Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
{
    auto* state = reinterpret_cast<DesignState*>(
        Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

    switch (msg)
    {
    case Win32::Messages::Size:
    {
        if (!state) break;
        Win32::RECT rc;
        Win32::GetClientRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        int canvasW = w - TOOLBOX_WIDTH - PROPERTY_WIDTH;
        if (canvasW < 0) canvasW = 0;
        Win32::MoveWindow(state->toolboxHwnd, 0, 0, TOOLBOX_WIDTH, h, true);
        Win32::MoveWindow(state->canvasHwnd, TOOLBOX_WIDTH, 0, canvasW, h, true);
        Win32::MoveWindow(state->propertyHwnd, w - PROPERTY_WIDTH, 0, PROPERTY_WIDTH, h, true);
        return 0;
    }

    case Win32::Messages::Command:
    {
        if (!state) break;

        if (Win32::GetLowWord(wParam) == IDC_TOOLBOX &&
            Win32::GetHighWord(wParam) == Win32::Notifications::ListBoxSelChange)
        {
            int sel = static_cast<int>(
                Win32::SendMessageW(state->toolboxHwnd, Win32::ListBox::GetCurSel, 0, 0));
            if (sel >= 0 && sel < static_cast<int>(std::size(TOOLBOX_ITEMS)))
            {
                state->placementMode = true;
                state->placementType = TOOLBOX_ITEMS[sel].type;
            }
            else
            {
                state->placementMode = false;
            }
            return 0;
        }

        switch (Win32::GetLowWord(wParam))
        {
        case IDM_FILE_NEW:     DoNew(*state);    return 0;
        case IDM_FILE_OPEN:    DoOpen(*state);   return 0;
        case IDM_FILE_SAVE:    DoSave(*state);   return 0;
        case IDM_FILE_SAVE_AS: DoSaveAs(*state); return 0;
        case IDM_FILE_EXIT:    Win32::SendMessageW(hwnd, Win32::Messages::Close, 0, 0); return 0;
        case IDM_CANCEL_PLACE: CancelPlacement(*state); return 0;
        }
        break;
    }

    case Win32::Messages::Close:
        if (state && !PromptSaveIfDirty(*state))
            return 0;
        Win32::DestroyWindow(hwnd);
        return 0;

    case Win32::Messages::NcDestroy:
        delete state;
        return 0;

    case Win32::Messages::Destroy:
        Win32::PostQuitMessage(0);
        return 0;
    }

    return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
}

}

export namespace Designer
{

auto CreateDesignSurface(
    Win32::HINSTANCE hInstance,
    FormDesigner::Form form,
    std::filesystem::path filePath = {}) -> Win32::HWND
{
    static bool registered = false;
    if (!registered)
    {
        Win32::WNDCLASSEXW wc = {
            .cbSize = sizeof(Win32::WNDCLASSEXW),
            .style = Win32::Cs_HRedraw | Win32::Cs_VRedraw,
            .lpfnWndProc = DesignSurfaceProc,
            .hInstance = hInstance,
            .hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
            .hbrBackground = reinterpret_cast<Win32::HBRUSH>(Win32::ColorBtnFace + 1),
            .lpszClassName = L"DesignSurface",
        };
        Win32::RegisterClassExW(&wc);

        Win32::WNDCLASSEXW canvasWc = {
            .cbSize = sizeof(Win32::WNDCLASSEXW),
            .style = Win32::Cs_HRedraw | Win32::Cs_VRedraw,
            .lpfnWndProc = CanvasProc,
            .hInstance = hInstance,
            .hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
            .hbrBackground = reinterpret_cast<Win32::HBRUSH>(Win32::ColorBtnFace + 1),
            .lpszClassName = L"DesignCanvas",
        };
        Win32::RegisterClassExW(&canvasWc);

        Win32::WNDCLASSEXW propWc = {
            .cbSize = sizeof(Win32::WNDCLASSEXW),
            .lpfnWndProc = PropertyPanelProc,
            .hInstance = hInstance,
            .hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
            .hbrBackground = reinterpret_cast<Win32::HBRUSH>(Win32::ColorBtnFace + 1),
            .lpszClassName = L"PropertyPanel",
        };
        Win32::RegisterClassExW(&propWc);

        registered = true;
    }

    auto* state = new DesignState{
        .form = std::move(form),
        .hInstance = hInstance,
        .currentFile = std::move(filePath),
    };

    auto menu = CreateMenuBar();

    Win32::RECT rc = { 0, 0,
        state->form.width + TOOLBOX_WIDTH + PROPERTY_WIDTH,
        state->form.height };
    Win32::AdjustWindowRectEx(&rc, Win32::Styles::OverlappedWindow, true, 0);

    auto hwnd = Win32::CreateWindowExW(
        0,
        L"DesignSurface",
        L"Form Designer",
        Win32::Styles::OverlappedWindow,
        Win32::Cw_UseDefault, Win32::Cw_UseDefault,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, menu, hInstance, nullptr);

    if (!hwnd)
    {
        delete state;
        return nullptr;
    }

    state->surfaceHwnd = hwnd;
    Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData,
        reinterpret_cast<Win32::LONG_PTR>(state));

    state->toolboxHwnd = Win32::CreateWindowExW(
        0, L"LISTBOX", nullptr,
        Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::Border |
            Win32::Styles::ListBoxNotify | Win32::Styles::ListBoxNoIntegralHeight,
        0, 0, TOOLBOX_WIDTH, state->form.height,
        hwnd,
        reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_TOOLBOX)),
        hInstance, nullptr);

    Win32::SendMessageW(state->toolboxHwnd, Win32::Messages::SetFont,
        reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);

    for (auto& item : TOOLBOX_ITEMS)
        Win32::SendMessageW(state->toolboxHwnd, Win32::ListBox::AddString, 0,
            reinterpret_cast<Win32::LPARAM>(item.name));

    state->canvasHwnd = Win32::CreateWindowExW(
        0, L"DesignCanvas", nullptr,
        Win32::Styles::Child | Win32::Styles::Visible,
        TOOLBOX_WIDTH, 0, state->form.width, state->form.height,
        hwnd, nullptr, hInstance, nullptr);

    Win32::SetWindowLongPtrW(state->canvasHwnd, Win32::Gwlp_UserData,
        reinterpret_cast<Win32::LONG_PTR>(state));

    state->propertyHwnd = Win32::CreateWindowExW(
        0, L"PropertyPanel", nullptr,
        Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::Border,
        state->form.width + TOOLBOX_WIDTH, 0,
        PROPERTY_WIDTH, state->form.height,
        hwnd, nullptr, hInstance, nullptr);

    Win32::SetWindowLongPtrW(state->propertyHwnd, Win32::Gwlp_UserData,
        reinterpret_cast<Win32::LONG_PTR>(state));

    CreatePropertyControls(*state);

    UpdateTitle(*state);
    PopulateControls(*state);
    UpdatePropertyPanel(*state);

    Win32::ShowWindow(hwnd, Win32::Sw_ShowDefault);
    Win32::UpdateWindow(hwnd);

    return hwnd;
}

auto RunDesignerLoop(Win32::HWND hwnd) -> int
{
    auto accel = CreateAcceleratorTable();
    Win32::MSG msg = {};
    while (Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        if (!Win32::TranslateAcceleratorW(hwnd, accel, &msg))
        {
            Win32::TranslateMessage(&msg);
            Win32::DispatchMessageW(&msg);
        }
    }
    Win32::DestroyAcceleratorTable(accel);
    return static_cast<int>(msg.wParam);
}

}
