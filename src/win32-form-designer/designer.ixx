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
	constexpr UINT IDM_FILE_NEW = 40001;
	constexpr UINT IDM_FILE_OPEN = 40002;
	constexpr UINT IDM_FILE_SAVE = 40003;
	constexpr UINT IDM_FILE_SAVE_AS = 40004;
	constexpr UINT IDM_FILE_EXIT = 40005;
	constexpr UINT IDM_CANCEL_PLACE = 40006;

	// Toolbox and layout constants.
	constexpr int TOOLBOX_WIDTH = 140;
	constexpr int PROPERTY_WIDTH = 220;
	constexpr UINT IDC_TOOLBOX = 50001;

	// Property panel edit control IDs.
	constexpr UINT IDC_PROP_TYPE = 51001;
	constexpr UINT IDC_PROP_TEXT = 51002;
	constexpr UINT IDC_PROP_ID = 51003;
	constexpr UINT IDC_PROP_X = 51004;
	constexpr UINT IDC_PROP_Y = 51005;
	constexpr UINT IDC_PROP_W = 51006;
	constexpr UINT IDC_PROP_H = 51007;
	constexpr UINT IDC_PROP_ONCLICK = 51008;

	struct ControlEntry
	{
		FormDesigner::Control* control;
		HWND hwnd;
	};

	enum class DragMode { None, Move, Resize };

	constexpr int HANDLE_SIZE = 6;
	constexpr int HANDLE_HALF = HANDLE_SIZE / 2;
	constexpr int MIN_CONTROL_SIZE = 10;

	// Control types available in the toolbox.
	struct ToolboxItem
	{
		FormDesigner::ControlType type;
		const wchar_t* name;
	};

	constexpr ToolboxItem TOOLBOX_ITEMS[] = {
	{ FormDesigner::ControlType::Button,   L"Button" },
	{ FormDesigner::ControlType::CheckBox, L"CheckBox" },
	{ FormDesigner::ControlType::Label,    L"Label" },
	{ FormDesigner::ControlType::TextBox,  L"TextBox" },
	{ FormDesigner::ControlType::GroupBox, L"GroupBox" },
	{ FormDesigner::ControlType::ListBox,  L"ListBox" },
	{ FormDesigner::ControlType::ComboBox, L"ComboBox" },
	};

	struct DesignState
	{
		FormDesigner::Form form;
		HINSTANCE hInstance = nullptr;
		HWND surfaceHwnd = nullptr;
		HWND canvasHwnd = nullptr;
		HWND toolboxHwnd = nullptr;
		HWND propertyHwnd = nullptr;
		std::vector<ControlEntry> entries;
		int selectedIndex = -1;

		DragMode dragMode = DragMode::None;
		int activeHandle = -1;
		POINT dragStart = {};
		POINT controlStart = {};
		SIZE controlStartSize = {};

		// Placement mode: active when a toolbox item is clicked.
		bool placementMode = false;
		FormDesigner::ControlType placementType = FormDesigner::ControlType::Button;

		// Prevents property panel notifications during programmatic updates.
		bool updatingProperties = false;

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
		for (int i = static_cast<int>(state.entries.size()) - 1; i >= 0; --i)
		{
			auto& r = state.entries[i].control->rect;
			if (x >= r.x && x < r.x + r.width &&
				y >= r.y && y < r.y + r.height)
				return i;
		}
		return -1;
	}

	// Computes the 8 handle anchor positions for a control rect.
	// Order: TL, TC, TR, ML, MR, BL, BC, BR
	void GetHandleAnchors(const FormDesigner::Rect& r, POINT out[8])
	{
		int cx = r.x + r.width / 2;
		int cy = r.y + r.height / 2;
		int rx = r.x + r.width;
		int by = r.y + r.height;

		out[0] = { r.x - HANDLE_HALF, r.y - HANDLE_HALF };
		out[1] = { cx - HANDLE_HALF, r.y - HANDLE_HALF };
		out[2] = { rx - HANDLE_HALF, r.y - HANDLE_HALF };
		out[3] = { r.x - HANDLE_HALF, cy - HANDLE_HALF };
		out[4] = { rx - HANDLE_HALF, cy - HANDLE_HALF };
		out[5] = { r.x - HANDLE_HALF, by - HANDLE_HALF };
		out[6] = { cx - HANDLE_HALF, by - HANDLE_HALF };
		out[7] = { rx - HANDLE_HALF, by - HANDLE_HALF };
	}

	// Returns handle index 0-7 if point is on a handle of the selected control, else -1.
	auto HitTestHandle(const DesignState& state, int x, int y) -> int
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

	auto CursorForHandle(int handle) -> LPCWSTR
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

	void ApplyResize(FormDesigner::Rect& r, int handle, int dx, int dy,
		const POINT& startPos, const SIZE& startSize)
	{
		bool moveLeft = (handle == 0 || handle == 3 || handle == 5);
		bool moveTop = (handle == 0 || handle == 1 || handle == 2);
		bool moveRight = (handle == 2 || handle == 4 || handle == 7);
		bool moveBottom = (handle == 5 || handle == 6 || handle == 7);

		int newX = startPos.x;
		int newY = startPos.y;
		int newW = startSize.cx;
		int newH = startSize.cy;

		if (moveLeft) { newX += dx; newW -= dx; }
		if (moveTop) { newY += dy; newH -= dy; }
		if (moveRight) { newW += dx; }
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

	void DrawSelection(const DesignState& state, HDC hdc)
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

	auto ControlTypeDisplayName(FormDesigner::ControlType type) -> const wchar_t*
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

	// Populates or clears the property panel based on the current selection.
	void UpdatePropertyPanel(DesignState& state)
	{
		if (!state.propertyHwnd) return;
		state.updatingProperties = true;

		auto panel = state.propertyHwnd;
		bool hasSel = state.selectedIndex >= 0 &&
			state.selectedIndex < static_cast<int>(state.entries.size());

		if (hasSel)
		{
			auto& ctrl = *state.entries[state.selectedIndex].control;

			SetDlgItemTextW(panel, IDC_PROP_TYPE, ControlTypeDisplayName(ctrl.type));
			SetDlgItemTextW(panel, IDC_PROP_TEXT, ctrl.text.c_str());
			SetDlgItemInt(panel, IDC_PROP_ID, static_cast<UINT>(ctrl.id), FALSE);
			SetDlgItemInt(panel, IDC_PROP_X, ctrl.rect.x, TRUE);
			SetDlgItemInt(panel, IDC_PROP_Y, ctrl.rect.y, TRUE);
			SetDlgItemInt(panel, IDC_PROP_W, ctrl.rect.width, TRUE);
			SetDlgItemInt(panel, IDC_PROP_H, ctrl.rect.height, TRUE);

			auto onClick = std::wstring(ctrl.onClick.begin(), ctrl.onClick.end());
			SetDlgItemTextW(panel, IDC_PROP_ONCLICK, onClick.c_str());
		}
		else
		{
			UINT ids[] = { IDC_PROP_TYPE, IDC_PROP_TEXT, IDC_PROP_ID,
			IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H, IDC_PROP_ONCLICK };
			for (auto id : ids)
				SetDlgItemTextW(panel, id, L"");
		}

		// Enable or disable editable fields based on selection.
		UINT editableIds[] = { IDC_PROP_TEXT, IDC_PROP_ID,
		IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H, IDC_PROP_ONCLICK };
		for (auto id : editableIds)
			EnableWindow(GetDlgItem(panel, id), hasSel);

		state.updatingProperties = false;
	}

	void RebuildControls(DesignState& state)
	{
		for (auto& entry : state.entries)
			DestroyWindow(entry.hwnd);
		state.entries.clear();
		state.selectedIndex = -1;
		PopulateControls(state);
		InvalidateRect(state.canvasHwnd, nullptr, TRUE);
		UpdatePropertyPanel(state);
	}

	auto NextControlId(const DesignState& state) -> int
	{
		int maxId = 0;
		for (auto& c : state.form.controls)
			if (c.id > maxId) maxId = c.id;
		return maxId + 1;
	}

	void PlaceControl(DesignState& state, int x, int y)
	{
		auto newId = NextControlId(state);
		auto& ctrl = state.form.controls.emplace_back();

		// Fix up existing entries' control pointers (vector may have reallocated).
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

	void CancelPlacement(DesignState& state)
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

		// Fix up control pointers invalidated by vector erase.
		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
			state.entries[i].control = &state.form.controls[i];

		state.selectedIndex = -1;
		MarkDirty(state);
		InvalidateRect(state.canvasHwnd, nullptr, TRUE);
		UpdatePropertyPanel(state);
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

		AppendMenuW(fileMenu, MF_STRING, IDM_FILE_NEW, L"&New\tCtrl+N");
		AppendMenuW(fileMenu, MF_STRING, IDM_FILE_OPEN, L"&Open...\tCtrl+O");
		AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
		AppendMenuW(fileMenu, MF_STRING, IDM_FILE_SAVE, L"&Save\tCtrl+S");
		AppendMenuW(fileMenu, MF_STRING, IDM_FILE_SAVE_AS, L"Save &As...\tCtrl+Shift+S");
		AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
		AppendMenuW(fileMenu, MF_STRING, IDM_FILE_EXIT, L"E&xit\tAlt+F4");

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

	// Reads a property edit value and applies it to the selected control.
	void ApplyPropertyChange(DesignState& state, UINT controlId)
	{
		if (state.selectedIndex < 0 ||
			state.selectedIndex >= static_cast<int>(state.entries.size()))
			return;

		auto& entry = state.entries[state.selectedIndex];
		auto& ctrl = *entry.control;
		auto panel = state.propertyHwnd;

		switch (controlId)
		{
		case IDC_PROP_TEXT:
		{
			wchar_t buf[512] = {};
			GetDlgItemTextW(panel, IDC_PROP_TEXT, buf, 512);
			ctrl.text = buf;
			SetWindowTextW(entry.hwnd, ctrl.text.c_str());
			break;
		}
		case IDC_PROP_ID:
		{
			BOOL ok = FALSE;
			auto val = GetDlgItemInt(panel, IDC_PROP_ID, &ok, FALSE);
			if (ok) ctrl.id = static_cast<int>(val);
			break;
		}
		case IDC_PROP_X:
		{
			BOOL ok = FALSE;
			auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_X, &ok, TRUE));
			if (ok) ctrl.rect.x = val;
			MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
				ctrl.rect.width, ctrl.rect.height, TRUE);
			InvalidateRect(state.canvasHwnd, nullptr, TRUE);
			break;
		}
		case IDC_PROP_Y:
		{
			BOOL ok = FALSE;
			auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_Y, &ok, TRUE));
			if (ok) ctrl.rect.y = val;
			MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
				ctrl.rect.width, ctrl.rect.height, TRUE);
			InvalidateRect(state.canvasHwnd, nullptr, TRUE);
			break;
		}
		case IDC_PROP_W:
		{
			BOOL ok = FALSE;
			auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_W, &ok, FALSE));
			if (ok && val >= MIN_CONTROL_SIZE) ctrl.rect.width = val;
			MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
				ctrl.rect.width, ctrl.rect.height, TRUE);
			InvalidateRect(state.canvasHwnd, nullptr, TRUE);
			break;
		}
		case IDC_PROP_H:
		{
			BOOL ok = FALSE;
			auto val = static_cast<int>(GetDlgItemInt(panel, IDC_PROP_H, &ok, FALSE));
			if (ok && val >= MIN_CONTROL_SIZE) ctrl.rect.height = val;
			MoveWindow(entry.hwnd, ctrl.rect.x, ctrl.rect.y,
				ctrl.rect.width, ctrl.rect.height, TRUE);
			InvalidateRect(state.canvasHwnd, nullptr, TRUE);
			break;
		}
		case IDC_PROP_ONCLICK:
		{
			wchar_t buf[256] = {};
			GetDlgItemTextW(panel, IDC_PROP_ONCLICK, buf, 256);
			ctrl.onClick = std::string(buf, buf + wcslen(buf));
			break;
		}
		default:
			return; // Unknown field — don't mark dirty.
		}

		MarkDirty(state);
	}

	// Creates the label + edit control pairs inside the property panel.
	void CreatePropertyControls(DesignState& state)
	{
		auto parent = state.propertyHwnd;
		auto hInst = state.hInstance;
		auto font = reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT));

		// Header label.
		auto header = CreateWindowExW(0, L"STATIC", L"Properties",
			WS_CHILD | WS_VISIBLE | SS_LEFT,
			5, 5, 210, 20, parent, nullptr, hInst, nullptr);
		SendMessageW(header, WM_SETFONT, font, TRUE);

		struct PropRow
		{
			const wchar_t* label;
			UINT editId;
			DWORD extraStyle;
		};

		PropRow rows[] = {
		{ L"Type:",    IDC_PROP_TYPE,    ES_READONLY },
		{ L"Text:",    IDC_PROP_TEXT,    ES_AUTOHSCROLL },
		{ L"ID:",      IDC_PROP_ID,     ES_AUTOHSCROLL },
		{ L"X:",       IDC_PROP_X,      ES_AUTOHSCROLL },
		{ L"Y:",       IDC_PROP_Y,      ES_AUTOHSCROLL },
		{ L"Width:",   IDC_PROP_W,      ES_AUTOHSCROLL },
		{ L"Height:",  IDC_PROP_H,      ES_AUTOHSCROLL },
		{ L"onClick:", IDC_PROP_ONCLICK, ES_AUTOHSCROLL },
		};

		int y = 30;
		for (auto& row : rows)
		{
			auto lbl = CreateWindowExW(0, L"STATIC", row.label,
				WS_CHILD | WS_VISIBLE | SS_RIGHT,
				5, y + 2, 55, 18, parent, nullptr, hInst, nullptr);
			SendMessageW(lbl, WM_SETFONT, font, TRUE);

			auto edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | row.extraStyle,
				65, y, 150, 22, parent,
				reinterpret_cast<HMENU>(static_cast<UINT_PTR>(row.editId)),
				hInst, nullptr);
			SendMessageW(edit, WM_SETFONT, font, TRUE);
			EnableWindow(edit, FALSE);

			y += 26;
		}
	}

	// Handles edit notifications from the property panel controls.
	LRESULT CALLBACK PropertyPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		auto* state = reinterpret_cast<DesignState*>(
			GetWindowLongPtrW(hwnd, GWLP_USERDATA));

		switch (msg)
		{
		case WM_COMMAND:
		{
			if (!state || state->updatingProperties) break;
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				ApplyPropertyChange(*state, LOWORD(wParam));
				return 0;
			}
			break;
		}
		}

		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	// Handles painting, mouse interaction, and keyboard input on the form canvas.
	LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

		case WM_LBUTTONDOWN:
		{
			if (!state) break;
			SetFocus(hwnd);
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			// In placement mode, create a new control at the click position.
			if (state->placementMode)
			{
				PlaceControl(*state, x, y);
				return 0;
			}

			// Check handles on currently selected control first.
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

			// Otherwise, hit-test for a control body click.
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

			// Not dragging — update cursor for placement or handle hover.
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
			// Let WM_MOUSEMOVE handle cursor for the client area.
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

	// Handles menu commands, toolbox selection, and window layout.
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

			// Toolbox selection notification.
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
	// Creates a design surface window with a toolbox, form canvas, and property panel.
	// Controls are rendered as real Win32 controls for WYSIWYG fidelity but
	// are mouse-transparent — all interaction is handled by the canvas.
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

		// Size to fit the form canvas plus both side panels.
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

		// Create the toolbox panel.
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

		// Create the editing canvas where form controls live.
		state->canvasHwnd = CreateWindowExW(
			0, L"DesignCanvas", nullptr,
			WS_CHILD | WS_VISIBLE,
			TOOLBOX_WIDTH, 0, state->form.width, state->form.height,
			hwnd, nullptr, hInstance, nullptr);

		SetWindowLongPtrW(state->canvasHwnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(state));

		// Create the property panel.
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

	// Standard message loop for the design surface.
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