module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <Commdlg.h>
#include <windowsx.h>

export module designer;
import std;
import formbuilder;

namespace Designer
{
	// Menu command IDs.
	constexpr UINT IDM_FILE_NEW     = 40001;
	constexpr UINT IDM_FILE_OPEN    = 40002;
	constexpr UINT IDM_FILE_SAVE    = 40003;
	constexpr UINT IDM_FILE_SAVE_AS = 40004;
	constexpr UINT IDM_FILE_EXIT    = 40005;

	struct ControlEntry
	{
		FormDesigner::Control* control;
		HWND hwnd;
	};

	struct DesignState
	{
		FormDesigner::Form form;
		HINSTANCE hInstance = nullptr;
		HWND surfaceHwnd = nullptr;
		std::vector<ControlEntry> entries;
		int selectedIndex = -1;

		bool dragging = false;
		POINT dragStart = {};
		POINT controlStart = {};

		std::filesystem::path currentFile;
		bool dirty = false;
	};

	constexpr UINT SUBCLASS_ID = 1;

	// Makes child controls transparent to mouse input so the
	// design surface receives all clicks and drags.
	LRESULT CALLBACK ControlSubclassProc(
		HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
		UINT_PTR, DWORD_PTR)
	{
		if (msg == WM_NCHITTEST)
			return HTTRANSPARENT;

		return DefSubclassProc(hwnd, msg, wParam, lParam);
	}

	auto HitTest(const DesignState& state, int x, int y) -> int
	{
		// Reverse iterate for z-order (last drawn is on top).
		for (int i = static_cast<int>(state.entries.size()) - 1; i >= 0; --i)
		{
			auto& r = state.entries[i].control->rect;
			if (x >= r.x && x < r.x + r.width &&
				y >= r.y && y < r.y + r.height)
				return i;
		}
		return -1;
	}

	void DrawSelection(const DesignState& state, HDC hdc)
	{
		if (state.selectedIndex < 0 ||
			state.selectedIndex >= static_cast<int>(state.entries.size()))
			return;

		auto& r = state.entries[state.selectedIndex].control->rect;

		// 2px blue border around the selected control.
		auto accent = CreateSolidBrush(RGB(0, 120, 215));
		RECT sides[] = {
			{ r.x - 2, r.y - 2,          r.x + r.width + 2, r.y },
			{ r.x - 2, r.y + r.height,   r.x + r.width + 2, r.y + r.height + 2 },
			{ r.x - 2, r.y,              r.x,                r.y + r.height },
			{ r.x + r.width, r.y,        r.x + r.width + 2,  r.y + r.height },
		};
		for (auto& s : sides)
			FillRect(hdc, &s, accent);

		// 6px resize handles at corners and edge midpoints.
		constexpr int H = 6;
		constexpr int half = H / 2;
		int cx = r.x + r.width / 2;
		int cy = r.y + r.height / 2;
		int rx = r.x + r.width;
		int by = r.y + r.height;

		POINT anchors[] = {
			{ r.x - half, r.y - half },
			{ cx  - half, r.y - half },
			{ rx  - half, r.y - half },
			{ r.x - half, cy  - half },
			{ rx  - half, cy  - half },
			{ r.x - half, by  - half },
			{ cx  - half, by  - half },
			{ rx  - half, by  - half },
		};

		auto white = CreateSolidBrush(RGB(255, 255, 255));
		for (auto& a : anchors)
		{
			RECT outer = { a.x, a.y, a.x + H, a.y + H };
			RECT inner = { a.x + 1, a.y + 1, a.x + H - 1, a.y + H - 1 };
			FillRect(hdc, &outer, accent);
			FillRect(hdc, &inner, white);
		}
		DeleteObject(white);
		DeleteObject(accent);
	}

	void PopulateControls(DesignState& state)
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
				state.surfaceHwnd,
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

	void UpdateTitle(DesignState& state)
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

	void MarkDirty(DesignState& state)
	{
		if (!state.dirty)
		{
			state.dirty = true;
			UpdateTitle(state);
		}
	}

	void RebuildControls(DesignState& state)
	{
		for (auto& entry : state.entries)
			DestroyWindow(entry.hwnd);
		state.entries.clear();
		state.selectedIndex = -1;
		PopulateControls(state);
		InvalidateRect(state.surfaceHwnd, nullptr, TRUE);
	}

	auto ShowSaveDialog(HWND owner, std::filesystem::path& outPath) -> bool
	{
		wchar_t filename[MAX_PATH] = {};
		if (!outPath.empty())
			wcscpy_s(filename, outPath.wstring().c_str());

		OPENFILENAMEW ofn = {
			.lStructSize = sizeof(OPENFILENAMEW),
			.hwndOwner = owner,
			.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0",
			.lpstrFile = filename,
			.nMaxFile = MAX_PATH,
			.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
			.lpstrDefExt = L"json",
		};

		if (!GetSaveFileNameW(&ofn))
			return false;

		outPath = filename;
		return true;
	}

	auto ShowOpenDialog(HWND owner, std::filesystem::path& outPath) -> bool
	{
		wchar_t filename[MAX_PATH] = {};

		OPENFILENAMEW ofn = {
			.lStructSize = sizeof(OPENFILENAMEW),
			.hwndOwner = owner,
			.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0",
			.lpstrFile = filename,
			.nMaxFile = MAX_PATH,
			.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		};

		if (!GetOpenFileNameW(&ofn))
			return false;

		outPath = filename;
		return true;
	}

	// Returns true if it's safe to discard current form (saved or user chose to).
	auto PromptSaveIfDirty(DesignState& state) -> bool
	{
		if (!state.dirty)
			return true;

		auto result = MessageBoxW(state.surfaceHwnd,
			L"Save changes before continuing?",
			L"Form Designer",
			MB_YESNOCANCEL | MB_ICONQUESTION);

		if (result == IDCANCEL)
			return false;

		if (result == IDYES)
		{
			if (state.currentFile.empty())
			{
				if (!ShowSaveDialog(state.surfaceHwnd, state.currentFile))
					return false;
			}
			FormDesigner::SaveFormToFile(state.form, state.currentFile);
			state.dirty = false;
			UpdateTitle(state);
		}

		return true;
	}

	void DoSave(DesignState& state)
	{
		if (state.currentFile.empty())
		{
			if (!ShowSaveDialog(state.surfaceHwnd, state.currentFile))
				return;
		}
		FormDesigner::SaveFormToFile(state.form, state.currentFile);
		state.dirty = false;
		UpdateTitle(state);
	}

	void DoSaveAs(DesignState& state)
	{
		auto path = state.currentFile;
		if (!ShowSaveDialog(state.surfaceHwnd, path))
			return;
		state.currentFile = path;
		FormDesigner::SaveFormToFile(state.form, state.currentFile);
		state.dirty = false;
		UpdateTitle(state);
	}

	void DoOpen(DesignState& state)
	{
		if (!PromptSaveIfDirty(state))
			return;

		auto path = std::filesystem::path{};
		if (!ShowOpenDialog(state.surfaceHwnd, path))
			return;

		try
		{
			state.form = FormDesigner::LoadFormFromFile(path);
			state.currentFile = path;
			state.dirty = false;
			RebuildControls(state);
			UpdateTitle(state);
		}
		catch (const std::exception& ex)
		{
			auto msg = std::string{ "Failed to open file:\n" } + ex.what();
			auto wide = std::wstring(msg.begin(), msg.end());
			MessageBoxW(state.surfaceHwnd, wide.c_str(), L"Error", MB_OK | MB_ICONERROR);
		}
	}

	void DoNew(DesignState& state)
	{
		if (!PromptSaveIfDirty(state))
			return;

		state.form = FormDesigner::Form{};
		state.currentFile.clear();
		state.dirty = false;
		RebuildControls(state);
		UpdateTitle(state);
	}

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
		};
		return CreateAcceleratorTableW(accels, static_cast<int>(std::size(accels)));
	}

	LRESULT CALLBACK DesignSurfaceProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		auto* state = reinterpret_cast<DesignState*>(
			GetWindowLongPtrW(hwnd, GWLP_USERDATA));

		switch (msg)
		{
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

		case WM_COMMAND:
		{
			if (!state) break;
			switch (LOWORD(wParam))
			{
			case IDM_FILE_NEW:     DoNew(*state);    return 0;
			case IDM_FILE_OPEN:    DoOpen(*state);   return 0;
			case IDM_FILE_SAVE:    DoSave(*state);   return 0;
			case IDM_FILE_SAVE_AS: DoSaveAs(*state); return 0;
			case IDM_FILE_EXIT:    SendMessageW(hwnd, WM_CLOSE, 0, 0); return 0;
			}
			break;
		}

		case WM_LBUTTONDOWN:
		{
			if (!state) break;
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			int hit = HitTest(*state, x, y);

			state->selectedIndex = hit;
			InvalidateRect(hwnd, nullptr, TRUE);

			if (hit >= 0)
			{
				auto& r = state->entries[hit].control->rect;
				state->dragging = true;
				state->dragStart = { x, y };
				state->controlStart = { r.x, r.y };
				SetCapture(hwnd);
			}
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			if (!state || !state->dragging) break;
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			auto& entry = state->entries[state->selectedIndex];
			entry.control->rect.x = state->controlStart.x + (x - state->dragStart.x);
			entry.control->rect.y = state->controlStart.y + (y - state->dragStart.y);

			MoveWindow(entry.hwnd,
				entry.control->rect.x, entry.control->rect.y,
				entry.control->rect.width, entry.control->rect.height,
				TRUE);

			InvalidateRect(hwnd, nullptr, TRUE);
			MarkDirty(*state);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			if (!state || !state->dragging) break;
			state->dragging = false;
			ReleaseCapture();
			return 0;
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
	// Creates a design surface window populated with controls from the form.
	// Controls are rendered as real Win32 controls for WYSIWYG fidelity but
	// are mouse-transparent — all interaction is handled by the surface.
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
				.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
				.lpszClassName = L"DesignSurface",
			};
			RegisterClassExW(&wc);
			registered = true;
		}

		auto* state = new DesignState{
			.form = std::move(form),
			.hInstance = hInstance,
			.currentFile = std::move(filePath),
		};

		auto menu = CreateMenuBar();

		RECT rc = { 0, 0, state->form.width, state->form.height };
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
		UpdateTitle(*state);

		PopulateControls(*state);

		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		return hwnd;
	}

	// Standard message loop for the design surface.
	auto RunDesignerLoop() -> int
	{
		auto accel = CreateAcceleratorTable();
		MSG msg = {};
		while (GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			if (!TranslateAcceleratorW(msg.hwnd, accel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		DestroyAcceleratorTable(accel);
		return static_cast<int>(msg.wParam);
	}
}
