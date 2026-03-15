module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>

export module designer;
import std;
import formbuilder;
export import :state;
export import :helpers;
export import :properties;
export import :canvas;
export import :fileops;

namespace Designer
{

auto CreateMenuBar() -> HMENU
{
    auto menuBar = CreateMenu();
    auto fileMenu = CreatePopupMenu();

    AppendMenuW(fileMenu, MF_STRING, IDM_FILE_NEW,     L"&New\tCtrl+N");
    AppendMenuW(fileMenu, MF_STRING, IDM_FILE_OPEN,    L"&Open...\tCtrl+O");
    AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(fileMenu, MF_STRING, IDM_FILE_SAVE,    L"&Save\tCtrl+S");
    AppendMenuW(fileMenu, MF_STRING, IDM_FILE_SAVE_AS, L"Save &As...\tCtrl+Shift+S");
    AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(fileMenu, MF_STRING, IDM_FILE_EXIT,    L"E&xit\tAlt+F4");

    AppendMenuW(menuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu), L"&File");
    return menuBar;
}

auto CreateAcceleratorTable() -> HACCEL
{
    ACCEL accels[] = {
        { FCONTROL | FVIRTKEY, 'N', static_cast<WORD>(IDM_FILE_NEW) },
        { FCONTROL | FVIRTKEY, 'O', static_cast<WORD>(IDM_FILE_OPEN) },
        { FCONTROL | FVIRTKEY, 'S', static_cast<WORD>(IDM_FILE_SAVE) },
        { FCONTROL | FSHIFT | FVIRTKEY, 'S', static_cast<WORD>(IDM_FILE_SAVE_AS) },
        { FVIRTKEY, VK_ESCAPE, static_cast<WORD>(IDM_CANCEL_PLACE) },
    };
    return CreateAcceleratorTableW(accels, static_cast<int>(std::size(accels)));
}

LRESULT CALLBACK DesignSurfaceProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto* state = reinterpret_cast<DesignState*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_SIZE:
    {
        if (!state) break;
        RECT rc;
        GetClientRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        int canvasW = w - TOOLBOX_WIDTH - PROPERTY_WIDTH;
        if (canvasW < 0) canvasW = 0;
        MoveWindow(state->toolboxHwnd, 0, 0, TOOLBOX_WIDTH, h, TRUE);
        MoveWindow(state->canvasHwnd, TOOLBOX_WIDTH, 0, canvasW, h, TRUE);
        MoveWindow(state->propertyHwnd, w - PROPERTY_WIDTH, 0, PROPERTY_WIDTH, h, TRUE);
        return 0;
    }

    case WM_COMMAND:
    {
        if (!state) break;

        if (LOWORD(wParam) == IDC_TOOLBOX && HIWORD(wParam) == LBN_SELCHANGE)
        {
            int sel = static_cast<int>(
                SendMessageW(state->toolboxHwnd, LB_GETCURSEL, 0, 0));
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

        switch (LOWORD(wParam))
        {
        case IDM_FILE_NEW:     DoNew(*state);    return 0;
        case IDM_FILE_OPEN:    DoOpen(*state);   return 0;
        case IDM_FILE_SAVE:    DoSave(*state);   return 0;
        case IDM_FILE_SAVE_AS: DoSaveAs(*state); return 0;
        case IDM_FILE_EXIT:    SendMessageW(hwnd, WM_CLOSE, 0, 0); return 0;
        case IDM_CANCEL_PLACE: CancelPlacement(*state); return 0;
        }
        break;
    }

    case WM_CLOSE:
        if (state && !PromptSaveIfDirty(*state))
            return 0;
        DestroyWindow(hwnd);
        return 0;

    case WM_NCDESTROY:
        delete state;
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

}

export namespace Designer
{

auto CreateDesignSurface(
    HINSTANCE hInstance,
    FormDesigner::Form form,
    std::filesystem::path filePath = {}) -> HWND
{
    static bool registered = false;
    if (!registered)
    {
        WNDCLASSEXW wc = {
            .cbSize = sizeof(WNDCLASSEXW),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = DesignSurfaceProc,
            .hInstance = hInstance,
            .hCursor = LoadCursorW(nullptr, IDC_ARROW),
            .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1),
            .lpszClassName = L"DesignSurface",
        };
        RegisterClassExW(&wc);

        WNDCLASSEXW canvasWc = {
            .cbSize = sizeof(WNDCLASSEXW),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = CanvasProc,
            .hInstance = hInstance,
            .hCursor = LoadCursorW(nullptr, IDC_ARROW),
            .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1),
            .lpszClassName = L"DesignCanvas",
        };
        RegisterClassExW(&canvasWc);

        WNDCLASSEXW propWc = {
            .cbSize = sizeof(WNDCLASSEXW),
            .lpfnWndProc = PropertyPanelProc,
            .hInstance = hInstance,
            .hCursor = LoadCursorW(nullptr, IDC_ARROW),
            .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1),
            .lpszClassName = L"PropertyPanel",
        };
        RegisterClassExW(&propWc);

        registered = true;
    }

    auto* state = new DesignState{
        .form = std::move(form),
        .hInstance = hInstance,
        .currentFile = std::move(filePath),
    };

    auto menu = CreateMenuBar();

    RECT rc = { 0, 0,
        state->form.width + TOOLBOX_WIDTH + PROPERTY_WIDTH,
        state->form.height };
    AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, 0);

    auto hwnd = CreateWindowExW(
        0,
        L"DesignSurface",
        L"Form Designer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, menu, hInstance, nullptr);

    if (!hwnd)
    {
        delete state;
        return nullptr;
    }

    state->surfaceHwnd = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

    state->toolboxHwnd = CreateWindowExW(
        0, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        0, 0, TOOLBOX_WIDTH, state->form.height,
        hwnd,
        reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_TOOLBOX)),
        hInstance, nullptr);

    SendMessageW(state->toolboxHwnd, WM_SETFONT,
        reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);

    for (auto& item : TOOLBOX_ITEMS)
        SendMessageW(state->toolboxHwnd, LB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(item.name));

    state->canvasHwnd = CreateWindowExW(
        0, L"DesignCanvas", nullptr,
        WS_CHILD | WS_VISIBLE,
        TOOLBOX_WIDTH, 0, state->form.width, state->form.height,
        hwnd, nullptr, hInstance, nullptr);

    SetWindowLongPtrW(state->canvasHwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(state));

    state->propertyHwnd = CreateWindowExW(
        0, L"PropertyPanel", nullptr,
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        state->form.width + TOOLBOX_WIDTH, 0,
        PROPERTY_WIDTH, state->form.height,
        hwnd, nullptr, hInstance, nullptr);

    SetWindowLongPtrW(state->propertyHwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(state));

    CreatePropertyControls(*state);

    UpdateTitle(*state);
    PopulateControls(*state);
    UpdatePropertyPanel(*state);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    return hwnd;
}

auto RunDesignerLoop(HWND hwnd) -> int
{
    auto accel = CreateAcceleratorTable();
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        if (!TranslateAcceleratorW(hwnd, accel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    DestroyAcceleratorTable(accel);
    return static_cast<int>(msg.wParam);
}

}
