module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>

export module designer;
import std;
import formbuilder;

namespace Designer
{
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

			// Draw selection handles on top of controls.
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
			return 0;
		}

		case WM_LBUTTONUP:
		{
			if (!state || !state->dragging) break;
			state->dragging = false;
			ReleaseCapture();
			return 0;
		}

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
	auto CreateDesignSurface(HINSTANCE hInstance, FormDesigner::Form form) -> HWND
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
		};

		RECT rc = { 0, 0, state->form.width, state->form.height };
		AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);

		auto hwnd = CreateWindowExW(
			0,
			L"DesignSurface",
			L"Form Designer",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top,
			nullptr, nullptr, hInstance, nullptr);

		if (!hwnd)
		{
			delete state;
			return nullptr;
		}

		state->surfaceHwnd = hwnd;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

		PopulateControls(*state);

		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		return hwnd;
	}

	// Standard message loop for the design surface.
	auto RunDesignerLoop() -> int
	{
		MSG msg = {};
		while (GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return static_cast<int>(msg.wParam);
	}
}
