export module designer;
import std;
import formbuilder;
export import :state;
export import :hit_testing;
export import :rendering;
export import :settings;
export import :helpers;
export import :properties;
export import :canvas;
export import :fileops;
export import :alignment;

namespace Designer
{

	void ApplyTheme(DesignState& state);

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
		Win32::AppendMenuW(fileMenu, Win32::Menu::String, IDM_FILE_PREVIEW, L"&Preview\tF5");
		Win32::AppendMenuW(fileMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(fileMenu, Win32::Menu::String, IDM_FILE_EXPORT_CPP, L"E&xport to C++...");
		Win32::AppendMenuW(fileMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(fileMenu, Win32::Menu::String, IDM_FILE_EXIT,    L"E&xit\tAlt+F4");

		Win32::AppendMenuW(menuBar, Win32::Menu::Popup,
			reinterpret_cast<Win32::UINT_PTR>(fileMenu), L"&File");

		auto editMenu = Win32::CreatePopupMenu();
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_UNDO,      L"&Undo\tCtrl+Z");
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_REDO,      L"&Redo\tCtrl+Y");
		Win32::AppendMenuW(editMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_CUT,       L"Cu&t\tCtrl+X");
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_COPY,      L"&Copy\tCtrl+C");
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_PASTE,     L"&Paste\tCtrl+V");
		Win32::AppendMenuW(editMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_DUPLICATE, L"&Duplicate\tCtrl+D");
		Win32::AppendMenuW(editMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_DELETE,    L"De&lete\tDel");
		Win32::AppendMenuW(editMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_SELECTALL, L"Select &All\tCtrl+A");
		Win32::AppendMenuW(editMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_GROUP,   L"&Group\tCtrl+G");
		Win32::AppendMenuW(editMenu, Win32::Menu::String, IDM_EDIT_UNGROUP, L"U&ngroup\tCtrl+Shift+G");

		Win32::AppendMenuW(menuBar, Win32::Menu::Popup,
			reinterpret_cast<Win32::UINT_PTR>(editMenu), L"&Edit");

		auto formatMenu = Win32::CreatePopupMenu();
		auto alignMenu = Win32::CreatePopupMenu();
		Win32::AppendMenuW(alignMenu, Win32::Menu::String, IDM_FORMAT_ALIGN_LEFT,     L"Align &Left");
		Win32::AppendMenuW(alignMenu, Win32::Menu::String, IDM_FORMAT_ALIGN_CENTER_H, L"Align &Center Horizontally");
		Win32::AppendMenuW(alignMenu, Win32::Menu::String, IDM_FORMAT_ALIGN_RIGHT,    L"Align &Right");
		Win32::AppendMenuW(alignMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(alignMenu, Win32::Menu::String, IDM_FORMAT_ALIGN_TOP,      L"Align &Top");
		Win32::AppendMenuW(alignMenu, Win32::Menu::String, IDM_FORMAT_ALIGN_MIDDLE_V, L"Align &Middle Vertically");
		Win32::AppendMenuW(alignMenu, Win32::Menu::String, IDM_FORMAT_ALIGN_BOTTOM,   L"Align &Bottom");
		Win32::AppendMenuW(formatMenu, Win32::Menu::Popup,
			reinterpret_cast<Win32::UINT_PTR>(alignMenu), L"&Align");

		auto distMenu = Win32::CreatePopupMenu();
		Win32::AppendMenuW(distMenu, Win32::Menu::String, IDM_FORMAT_DIST_HORIZ, L"Distribute &Horizontally");
		Win32::AppendMenuW(distMenu, Win32::Menu::String, IDM_FORMAT_DIST_VERT,  L"Distribute &Vertically");
		Win32::AppendMenuW(formatMenu, Win32::Menu::Popup,
			reinterpret_cast<Win32::UINT_PTR>(distMenu), L"&Distribute");

		auto sizeMenu = Win32::CreatePopupMenu();
		Win32::AppendMenuW(sizeMenu, Win32::Menu::String, IDM_FORMAT_SAME_WIDTH,  L"Same &Width");
		Win32::AppendMenuW(sizeMenu, Win32::Menu::String, IDM_FORMAT_SAME_HEIGHT, L"Same &Height");
		Win32::AppendMenuW(sizeMenu, Win32::Menu::String, IDM_FORMAT_SAME_SIZE,   L"Same &Size");
		Win32::AppendMenuW(formatMenu, Win32::Menu::Popup,
			reinterpret_cast<Win32::UINT_PTR>(sizeMenu), L"&Size");

		Win32::AppendMenuW(menuBar, Win32::Menu::Popup,
			reinterpret_cast<Win32::UINT_PTR>(formatMenu), L"F&ormat");

		auto viewMenu = Win32::CreatePopupMenu();
		Win32::AppendMenuW(viewMenu, Win32::Menu::String | Win32::Menu::Checked,
			IDM_VIEW_SHOWGRID, L"Show &Grid");
		Win32::AppendMenuW(viewMenu, Win32::Menu::String | Win32::Menu::Checked,
			IDM_VIEW_SNAPTOGRID, L"&Snap to Grid");
		Win32::AppendMenuW(viewMenu, Win32::Menu::String | Win32::Menu::Checked,
			IDM_VIEW_SHOWRULERS, L"Show &Rulers");
		Win32::AppendMenuW(viewMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(viewMenu, Win32::Menu::String, IDM_VIEW_CLEARGUIDES, L"&Clear All Guides");
		Win32::AppendMenuW(viewMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(viewMenu, Win32::Menu::String, IDM_VIEW_DARKMODE, L"&Dark Mode");
		Win32::AppendMenuW(viewMenu, Win32::Menu::Separator, 0, nullptr);
		Win32::AppendMenuW(viewMenu, Win32::Menu::String, IDM_VIEW_TABORDER, L"Edit &Tab Order");
		Win32::AppendMenuW(viewMenu, Win32::Menu::String, IDM_VIEW_ZORDER, L"Tab && &Z-Order...");

		Win32::AppendMenuW(menuBar, Win32::Menu::Popup,
			reinterpret_cast<Win32::UINT_PTR>(viewMenu), L"&View");

		return menuBar;
	}

	auto CreateAcceleratorTable() -> Win32::HACCEL
	{
		Win32::ACCEL accels[] = {
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'N', static_cast<Win32::WORD>(IDM_FILE_NEW) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'O', static_cast<Win32::WORD>(IDM_FILE_OPEN) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'S', static_cast<Win32::WORD>(IDM_FILE_SAVE) },
			{ Win32::Accel::Control | Win32::Accel::Shift | Win32::Accel::VirtKey, 'S', static_cast<Win32::WORD>(IDM_FILE_SAVE_AS) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'Z', static_cast<Win32::WORD>(IDM_EDIT_UNDO) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'Y', static_cast<Win32::WORD>(IDM_EDIT_REDO) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'X', static_cast<Win32::WORD>(IDM_EDIT_CUT) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'C', static_cast<Win32::WORD>(IDM_EDIT_COPY) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'V', static_cast<Win32::WORD>(IDM_EDIT_PASTE) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'D', static_cast<Win32::WORD>(IDM_EDIT_DUPLICATE) },
			{ Win32::Accel::Control | Win32::Accel::VirtKey, 'G', static_cast<Win32::WORD>(IDM_EDIT_GROUP) },
			{ Win32::Accel::Control | Win32::Accel::Shift | Win32::Accel::VirtKey, 'G', static_cast<Win32::WORD>(IDM_EDIT_UNGROUP) },
			{ Win32::Accel::VirtKey, static_cast<Win32::WORD>(Win32::Keys::Escape), static_cast<Win32::WORD>(IDM_CANCEL_PLACE) },
			{ Win32::Accel::VirtKey, static_cast<Win32::WORD>(Win32::Keys::F5), static_cast<Win32::WORD>(IDM_FILE_PREVIEW) },
		};
		return Win32::CreateAcceleratorTableW(accels, static_cast<int>(std::size(accels)));
	}

	constexpr int BASE_ZORDER_WIDTH  = 260;
	constexpr int BASE_ZORDER_HEIGHT = 340;
	constexpr int BASE_ZORDER_BTN_H  = 28;
	constexpr int BASE_ZORDER_BTN_W  = 115;
	constexpr int BASE_ZORDER_PAD    = 6;

	auto ZOrderPanelProc(Win32::HWND hwnd, Win32::UINT msg,
		Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
	{
		auto* state = reinterpret_cast<DesignState*>(
			Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

		switch (msg)
		{
		case Win32::Messages::Command:
		{
			if (!state) break;
			auto id = Win32::GetLowWord(wParam);
			auto code = Win32::GetHighWord(wParam);

			if (id == IDC_ZORDER_LIST && code == Win32::Notifications::ListBoxSelChange)
			{
				int sel = static_cast<int>(
					Win32::SendMessageW(Win32::GetDlgItem(hwnd, IDC_ZORDER_LIST),
						Win32::ListBox::GetCurSel, 0, 0));
				if (sel >= 0 && sel < static_cast<int>(state->entries.size()))
				{
					state->selection = { sel };
					Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
					UpdatePropertyPanel(*state);
				}
				return 0;
			}

			if (code == Win32::Notifications::ButtonClicked)
			{
				auto list = Win32::GetDlgItem(hwnd, IDC_ZORDER_LIST);
				int sel = static_cast<int>(Win32::SendMessageW(list, Win32::ListBox::GetCurSel, 0, 0));
				if (sel < 0) return 0;

				int count = static_cast<int>(state->form.controls.size());
				int newSel = sel;

				switch (id)
				{
				case IDC_ZORDER_UP:     if (sel > 0) newSel = sel - 1; break;
				case IDC_ZORDER_DOWN:   if (sel < count - 1) newSel = sel + 1; break;
				case IDC_ZORDER_TOP:    newSel = 0; break;
				case IDC_ZORDER_BOTTOM: newSel = count - 1; break;
				}

				if (newSel != sel)
				{
					MoveControlInZOrder(*state, sel, newSel);
					Win32::SendMessageW(list, Win32::ListBox::SetCurSel,
						static_cast<Win32::WPARAM>(newSel), 0);
					state->selection = { newSel };
					Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
					UpdatePropertyPanel(*state);
				}
				return 0;
			}
			break;
		}

		case Win32::Messages::Close:
			state->zorderHwnd = nullptr;
			Win32::DestroyWindow(hwnd);
			return 0;

		case Win32::Messages::NcDestroy:
			return 0;
		}

		return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	void ShowZOrderPanel(DesignState& state)
	{
		if (state.zorderHwnd)
		{
			Win32::SetFocus(state.zorderHwnd);
			return;
		}

		// Position to the right of the main window.
		Win32::RECT rc;
		Win32::GetWindowRect(state.surfaceHwnd, &rc);

		auto& d = state.dpiInfo;
		int zw   = d.Scale(BASE_ZORDER_WIDTH);
		int zh   = d.Scale(BASE_ZORDER_HEIGHT);
		int zbh  = d.Scale(BASE_ZORDER_BTN_H);
		int zbw  = d.Scale(BASE_ZORDER_BTN_W);
		int zpad = d.Scale(BASE_ZORDER_PAD);

		state.zorderHwnd = Win32::CreateWindowExW(
			0, L"ZOrderPanel", L"Tab & Z-Order",
			Win32::Styles::OverlappedWindow & ~Win32::Styles::ThickFrame & ~Win32::Styles::MaximizeBox,
			rc.right + 4, rc.top, zw, zh,
			state.surfaceHwnd, nullptr, state.hInstance, nullptr);

		Win32::SetWindowLongPtrW(state.zorderHwnd, Win32::Gwlp_UserData,
			reinterpret_cast<Win32::LONG_PTR>(&state));

		auto font = reinterpret_cast<Win32::WPARAM>(
			Win32::GetStockObject(Win32::DefaultGuiFont));

		Win32::RECT clientRc;
		Win32::GetClientRect(state.zorderHwnd, &clientRc);
		int cw = clientRc.right;
		int ch = clientRc.bottom;
		int listH = ch - 2 * (zbh + zpad) - zpad * 2;

		auto list = Win32::CreateWindowExW(
			Win32::ExStyles::ClientEdge, L"LISTBOX", nullptr,
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::Border |
				Win32::Styles::ListBoxNotify | Win32::Styles::ListBoxNoIntegralHeight |
				Win32::Styles::VScroll,
			zpad, zpad, cw - zpad * 2, listH,
			state.zorderHwnd,
			reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_ZORDER_LIST)),
			state.hInstance, nullptr);
		Win32::SendMessageW(list, Win32::Messages::SetFont, font, true);

		int btnY = zpad + listH + zpad;
		int btnX1 = zpad;
		int btnX2 = zpad + zbw + zpad;

		struct Btn { const wchar_t* text; Win32::UINT id; int x; int y; };
		Btn buttons[] = {
			{ L"\u25B2 Move Up",   IDC_ZORDER_UP,     btnX1, btnY },
			{ L"\u25BC Move Down", IDC_ZORDER_DOWN,   btnX2, btnY },
			{ L"\u25B2\u25B2 To Top", IDC_ZORDER_TOP, btnX1, btnY + zbh + zpad },
			{ L"\u25BC\u25BC To Bottom", IDC_ZORDER_BOTTOM, btnX2, btnY + zbh + zpad },
		};

		for (auto& btn : buttons)
		{
			auto hwnd = Win32::CreateWindowExW(0, L"BUTTON", btn.text,
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
				btn.x, btn.y, zbw, zbh,
				state.zorderHwnd,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(btn.id)),
				state.hInstance, nullptr);
			Win32::SendMessageW(hwnd, Win32::Messages::SetFont, font, true);
		}

		// Populate.
		RefreshZOrderPanel(state);

		Win32::ShowWindow(state.zorderHwnd, Win32::Sw_ShowDefault);
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

			// Let toolbar and statusbar auto-resize.
			if (state->toolbarHwnd)
				Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::AutoSize, 0, 0);
			if (state->statusbarHwnd)
				Win32::SendMessageW(state->statusbarHwnd, Win32::Messages::Size, 0, 0);

			// Query bar heights.
			int tbH = 0, sbH = 0;
			if (state->toolbarHwnd)
			{
				Win32::RECT tbRc;
				Win32::GetWindowRect(state->toolbarHwnd, &tbRc);
				tbH = tbRc.bottom - tbRc.top;
			}
			if (state->statusbarHwnd)
			{
				Win32::RECT sbRc;
				Win32::GetWindowRect(state->statusbarHwnd, &sbRc);
				sbH = sbRc.bottom - sbRc.top;
			}

			Win32::RECT rc;
			Win32::GetClientRect(hwnd, &rc);
			int w = rc.right - rc.left;
			int h = rc.bottom - rc.top - tbH - sbH;
			if (h < 0) h = 0;
			int tw = state->dpiInfo.ToolboxWidth();
			int pw = state->dpiInfo.PropertyWidth();
			int canvasW = w - tw - pw;
			if (canvasW < 0) canvasW = 0;
			Win32::MoveWindow(state->toolboxHwnd, 0, tbH, tw, h, true);
			Win32::MoveWindow(state->canvasHwnd, tw, tbH, canvasW, h, true);
			Win32::MoveWindow(state->propertyHwnd, w - pw, tbH, pw, h, true);

			// Update statusbar pane widths.
			if (state->statusbarHwnd)
			{
				int parts[] = { w / 2, w - 100, -1 };
				Win32::SendMessageW(state->statusbarHwnd, Win32::StatusBar::SetParts,
					3, reinterpret_cast<Win32::LPARAM>(parts));
			}
			return 0;
		}

		case Win32::Messages::DpiChanged:
		{
			if (!state) break;

			// Update DPI and resize to the system-suggested rect.
			state->dpiInfo.dpi = Win32::GetHighWord(wParam);
			auto* suggested = reinterpret_cast<Win32::RECT*>(lParam);
			Win32::SetWindowPos(hwnd, nullptr,
				suggested->left, suggested->top,
				suggested->right - suggested->left,
				suggested->bottom - suggested->top,
				Win32::Swp::NoZOrder | Win32::Swp::NoActivate);

			// Rebuild property panel controls for new DPI.
			Win32::EnumChildWindows(state->propertyHwnd,
				[](Win32::HWND child, Win32::LPARAM) -> Win32::BOOL {
					Win32::DestroyWindow(child);
					return true;
				}, 0);
			state->propertyScrollY = 0;
			CreatePropertyControls(*state);
			UpdatePropertyPanel(*state);

			Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
			return 0;
		}

		case Win32::Messages::InitMenuPopup:
		{
			if (!state) break;
			auto menu = reinterpret_cast<Win32::HMENU>(wParam);
			auto selCount = state->selection.size();
			auto flag2 = (selCount >= 2) ? Win32::Menu::Enabled : Win32::Menu::Grayed;
			auto flag3 = (selCount >= 3) ? Win32::Menu::Enabled : Win32::Menu::Grayed;

			Win32::EnableMenuItem(menu, IDM_FORMAT_ALIGN_LEFT,     Win32::Menu::ByCommand | flag2);
			Win32::EnableMenuItem(menu, IDM_FORMAT_ALIGN_CENTER_H, Win32::Menu::ByCommand | flag2);
			Win32::EnableMenuItem(menu, IDM_FORMAT_ALIGN_RIGHT,    Win32::Menu::ByCommand | flag2);
			Win32::EnableMenuItem(menu, IDM_FORMAT_ALIGN_TOP,      Win32::Menu::ByCommand | flag2);
			Win32::EnableMenuItem(menu, IDM_FORMAT_ALIGN_MIDDLE_V, Win32::Menu::ByCommand | flag2);
			Win32::EnableMenuItem(menu, IDM_FORMAT_ALIGN_BOTTOM,   Win32::Menu::ByCommand | flag2);
			Win32::EnableMenuItem(menu, IDM_FORMAT_DIST_HORIZ,     Win32::Menu::ByCommand | flag3);
			Win32::EnableMenuItem(menu, IDM_FORMAT_DIST_VERT,      Win32::Menu::ByCommand | flag3);
			Win32::EnableMenuItem(menu, IDM_FORMAT_SAME_WIDTH,     Win32::Menu::ByCommand | flag2);
			Win32::EnableMenuItem(menu, IDM_FORMAT_SAME_HEIGHT,    Win32::Menu::ByCommand | flag2);
			Win32::EnableMenuItem(menu, IDM_FORMAT_SAME_SIZE,      Win32::Menu::ByCommand | flag2);

			// Update toolbar alignment buttons.
			Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::EnableButton,
				IDM_FORMAT_ALIGN_LEFT,     selCount >= 2 ? 1 : 0);
			Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::EnableButton,
				IDM_FORMAT_ALIGN_CENTER_H, selCount >= 2 ? 1 : 0);
			Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::EnableButton,
				IDM_FORMAT_ALIGN_RIGHT,    selCount >= 2 ? 1 : 0);
			Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::EnableButton,
				IDM_FORMAT_ALIGN_TOP,      selCount >= 2 ? 1 : 0);
			Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::EnableButton,
				IDM_FORMAT_ALIGN_MIDDLE_V, selCount >= 2 ? 1 : 0);
			Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::EnableButton,
				IDM_FORMAT_ALIGN_BOTTOM,   selCount >= 2 ? 1 : 0);
			break;
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
			case IDM_FILE_PREVIEW: PreviewForm(*state); return 0;
			case IDM_FILE_EXPORT_CPP: DoExportCpp(*state); return 0;
			default:
			{
				auto id = Win32::GetLowWord(wParam);
				if (id >= IDM_FILE_RECENT_BASE &&
					id < IDM_FILE_RECENT_BASE + MAX_RECENT_FILES)
				{
					auto idx = id - IDM_FILE_RECENT_BASE;
					if (idx < static_cast<int>(state->recentFiles.size()))
					{
						if (!PromptSaveIfDirty(*state))
							return 0;
						DoOpenFile(*state, state->recentFiles[idx]);
					}
					return 0;
				}
				break;
			}
			case IDM_EDIT_UNDO:      Undo(*state);             return 0;
			case IDM_EDIT_REDO:      Redo(*state);             return 0;
			case IDM_EDIT_CUT:       CutSelected(*state);      return 0;
			case IDM_EDIT_COPY:      CopySelected(*state);     return 0;
			case IDM_EDIT_PASTE:     PasteControl(*state);     return 0;
			case IDM_EDIT_DUPLICATE: DuplicateSelected(*state); return 0;
			case IDM_EDIT_DELETE:    DeleteSelectedControls(*state); return 0;
			case IDM_EDIT_SELECTALL: SelectAll(*state);        return 0;
			case IDM_EDIT_GROUP:    GroupSelected(*state);     return 0;
			case IDM_EDIT_UNGROUP:  UngroupSelected(*state);  return 0;
			case IDM_CANCEL_PLACE: CancelPlacement(*state); return 0;

			// Format menu — alignment, distribution, sizing.
			case IDM_FORMAT_ALIGN_LEFT:
			case IDM_FORMAT_ALIGN_CENTER_H:
			case IDM_FORMAT_ALIGN_RIGHT:
			case IDM_FORMAT_ALIGN_TOP:
			case IDM_FORMAT_ALIGN_MIDDLE_V:
			case IDM_FORMAT_ALIGN_BOTTOM:
			case IDM_FORMAT_DIST_HORIZ:
			case IDM_FORMAT_DIST_VERT:
			case IDM_FORMAT_SAME_WIDTH:
			case IDM_FORMAT_SAME_HEIGHT:
			case IDM_FORMAT_SAME_SIZE:
			{
				auto ctrls = CollectUnlocked(state->form.controls, state->selection);
				if (ctrls.size() < 2) return 0;
				PushUndo(*state);
				auto fmtCmd = Win32::GetLowWord(wParam);
				switch (fmtCmd)
				{
				case IDM_FORMAT_ALIGN_LEFT:     AlignLeft(ctrls); break;
				case IDM_FORMAT_ALIGN_CENTER_H: AlignCenterH(ctrls); break;
				case IDM_FORMAT_ALIGN_RIGHT:    AlignRight(ctrls); break;
				case IDM_FORMAT_ALIGN_TOP:      AlignTop(ctrls); break;
				case IDM_FORMAT_ALIGN_MIDDLE_V: AlignMiddleV(ctrls); break;
				case IDM_FORMAT_ALIGN_BOTTOM:   AlignBottom(ctrls); break;
				case IDM_FORMAT_DIST_HORIZ:     DistributeHorizontally(ctrls); break;
				case IDM_FORMAT_DIST_VERT:      DistributeVertically(ctrls); break;
				case IDM_FORMAT_SAME_WIDTH:     MakeSameWidth(ctrls); break;
				case IDM_FORMAT_SAME_HEIGHT:    MakeSameHeight(ctrls); break;
				case IDM_FORMAT_SAME_SIZE:      MakeSameSize(ctrls); break;
				}
				int offset = RulerOffset(*state);
				for (int idx : state->selection)
				{
					auto& r = state->entries[idx].control->rect;
					Win32::MoveWindow(state->entries[idx].hwnd,
						r.x + offset, r.y + offset, r.width, r.height, true);
				}
				Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
				MarkDirty(*state);
				UpdatePropertyPanel(*state);
				return 0;
			}
			case IDM_VIEW_ZORDER:  ShowZOrderPanel(*state); return 0;
			case IDM_VIEW_TABORDER:
			{
				state->tabOrderMode = !state->tabOrderMode;
				auto menu = Win32::GetMenu(hwnd);
				Win32::CheckMenuItem(menu, IDM_VIEW_TABORDER,
					state->tabOrderMode ? Win32::Menu::Checked : Win32::Menu::Unchecked);
				if (state->tabOrderMode)
				{
					PushUndo(*state);
					AutoAssignTabOrder(*state);
					state->tabOrderNext = static_cast<int>(state->entries.size()) + 1;
				}
				else
				{
					state->tabOrderNext = 1;
				}
				Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
				return 0;
			}
			case IDM_VIEW_SHOWGRID:
			{
				state->showGrid = !state->showGrid;
				auto menu = Win32::GetMenu(hwnd);
				Win32::CheckMenuItem(menu, IDM_VIEW_SHOWGRID,
					state->showGrid ? Win32::Menu::Checked : Win32::Menu::Unchecked);
				Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
				return 0;
			}
			case IDM_VIEW_SNAPTOGRID:
			{
				state->snapToGrid = !state->snapToGrid;
				auto menu = Win32::GetMenu(hwnd);
				Win32::CheckMenuItem(menu, IDM_VIEW_SNAPTOGRID,
					state->snapToGrid ? Win32::Menu::Checked : Win32::Menu::Unchecked);
				return 0;
			}
			case IDM_VIEW_SHOWRULERS:
			{
				state->showRulers = !state->showRulers;
				auto menu = Win32::GetMenu(hwnd);
				Win32::CheckMenuItem(menu, IDM_VIEW_SHOWRULERS,
					state->showRulers ? Win32::Menu::Checked : Win32::Menu::Unchecked);
				RebuildControls(*state);
				return 0;
			}
			case IDM_VIEW_CLEARGUIDES:
			{
				state->userGuides.clear();
				Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
				return 0;
			}
			case IDM_VIEW_DARKMODE:
			{
				state->theme = state->theme.isDark ? LightTheme() : DarkTheme();
				auto menu = Win32::GetMenu(hwnd);
				Win32::CheckMenuItem(menu, IDM_VIEW_DARKMODE,
					state->theme.isDark ? Win32::Menu::Checked : Win32::Menu::Unchecked);
				ApplyTheme(*state);
				SaveThemePreference(state->theme.isDark);
				return 0;
			}
			}
			break;
		}

		case Win32::Messages::Close:
			if (state && !PromptSaveIfDirty(*state))
				return 0;
			Win32::DestroyWindow(hwnd);
			return 0;

		case Win32::Messages::NcDestroy:
			if (state && state->zorderHwnd)
				Win32::DestroyWindow(state->zorderHwnd);
			if (state && state->previewHwnd)
				Win32::DestroyWindow(state->previewHwnd);
			delete state;
			return 0;

		case Win32::Messages::Destroy:
			Win32::PostQuitMessage(0);
			return 0;
		}

		return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	void ApplyTheme(DesignState& state)
	{
		auto bgBrush = Win32::CreateSolidBrush(state.theme.panelBackground);

		// Update window class backgrounds for all panel types.
		Win32::SetClassBackground(state.surfaceHwnd, bgBrush);
		Win32::SetClassBackground(state.propertyHwnd, bgBrush);
		if (state.zorderHwnd)
			Win32::SetClassBackground(state.zorderHwnd, bgBrush);

		auto canvasBrush = Win32::CreateSolidBrush(state.theme.canvasBackground);
		Win32::SetClassBackground(state.canvasHwnd, canvasBrush);

		// Dark title bar (Windows 10 1809+).
		Win32::SetDarkTitleBar(state.surfaceHwnd, state.theme.isDark);

		// Dark scrollbars on scrollable panels.
		Win32::SetDarkScrollBars(state.propertyHwnd, state.theme.isDark);
		Win32::SetDarkScrollBars(state.toolboxHwnd, state.theme.isDark);

		// Repaint everything.
		Win32::InvalidateRect(state.surfaceHwnd, nullptr, true);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
		Win32::InvalidateRect(state.toolboxHwnd, nullptr, true);
		Win32::InvalidateRect(state.propertyHwnd, nullptr, true);
		Win32::InvalidateRect(state.toolbarHwnd, nullptr, true);
		Win32::InvalidateRect(state.statusbarHwnd, nullptr, true);
		if (state.zorderHwnd)
			Win32::InvalidateRect(state.zorderHwnd, nullptr, true);
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

			Win32::WNDCLASSEXW zorderWc = {
				.cbSize = sizeof(Win32::WNDCLASSEXW),
				.lpfnWndProc = ZOrderPanelProc,
				.hInstance = hInstance,
				.hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
				.hbrBackground = reinterpret_cast<Win32::HBRUSH>(Win32::ColorBtnFace + 1),
				.lpszClassName = L"ZOrderPanel",
			};
			Win32::RegisterClassExW(&zorderWc);

			registered = true;
		}

		auto* state = new DesignState{
			.form = std::move(form),
			.hInstance = hInstance,
			.currentFile = std::move(filePath),
		};
		state->dpiInfo.dpi = static_cast<int>(Win32::GetDpiForSystem());

		auto menu = CreateMenuBar();

		int tw = state->dpiInfo.ToolboxWidth();
		int pw = state->dpiInfo.PropertyWidth();
		Win32::RECT rc = { 0, 0,
			state->form.width + tw + pw,
			state->form.height };
		Win32::AdjustWindowRectExForDpi(&rc, Win32::Styles::OverlappedWindow, true, 0,
			static_cast<Win32::UINT>(state->dpiInfo.dpi));

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

		// Create toolbar.
		state->toolbarHwnd = Win32::CreateWindowExW(0,
			Win32::Controls::Toolbar, nullptr,
			Win32::Styles::Child | Win32::Styles::Visible |
				Win32::ToolbarStyle::Flat | Win32::ToolbarStyle::List |
				Win32::ToolbarStyle::Tooltips | Win32::CommonControlStyle::Top,
			0, 0, 0, 0, hwnd, nullptr, hInstance, nullptr);

		Win32::SendMessageW(state->toolbarHwnd,
			Win32::Toolbar::ButtonStructSize, sizeof(Win32::TBBUTTON), 0);

		// Hide bitmap area — text-only buttons.
		Win32::SendMessageW(state->toolbarHwnd,
			Win32::Toolbar::SetBitmapSize, 0, 0);

		// Toolbar button definitions reusing existing menu command IDs.
		static const wchar_t* btnLabels[] = {
			L"New", L"Open", L"Save", nullptr,
			L"Undo", L"Redo", nullptr,
			L"Cut", L"Copy", L"Paste", L"Delete", nullptr,
			L"Preview", nullptr,
			L"\x2190", L"\x2194", L"\x2192", L"\x2191", L"\x2195", L"\x2193"
		};
		Win32::UINT btnCmds[] = {
			IDM_FILE_NEW, IDM_FILE_OPEN, IDM_FILE_SAVE, 0,
			IDM_EDIT_UNDO, IDM_EDIT_REDO, 0,
			IDM_EDIT_CUT, IDM_EDIT_COPY, IDM_EDIT_PASTE, IDM_EDIT_DELETE, 0,
			IDM_FILE_PREVIEW, 0,
			IDM_FORMAT_ALIGN_LEFT, IDM_FORMAT_ALIGN_CENTER_H, IDM_FORMAT_ALIGN_RIGHT,
			IDM_FORMAT_ALIGN_TOP, IDM_FORMAT_ALIGN_MIDDLE_V, IDM_FORMAT_ALIGN_BOTTOM
		};

		constexpr int btnCount = 20;
		Win32::TBBUTTON buttons[btnCount] = {};
		for (int i = 0; i < btnCount; ++i)
		{
			if (btnLabels[i] == nullptr)
			{
				buttons[i].iBitmap = 0;
				buttons[i].fsStyle = Win32::ButtonStyle::Sep;
			}
			else
			{
				buttons[i].iBitmap = -1; // no bitmap, use I_IMAGENONE
				buttons[i].idCommand = static_cast<int>(btnCmds[i]);
				buttons[i].fsState = Win32::ButtonStyle::Enabled;
				buttons[i].fsStyle = Win32::ButtonStyle::Button | Win32::ButtonStyle::ShowText;
				buttons[i].iString = reinterpret_cast<Win32::INT_PTR>(btnLabels[i]);
			}
		}
		Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::AddButtons,
			btnCount, reinterpret_cast<Win32::LPARAM>(buttons));
		Win32::SendMessageW(state->toolbarHwnd, Win32::Toolbar::AutoSize, 0, 0);

		// Create status bar.
		state->statusbarHwnd = Win32::CreateWindowExW(0,
			Win32::Controls::StatusBar, nullptr,
			Win32::Styles::Child | Win32::Styles::Visible | Win32::StatusBarStyle::SizeGrip,
			0, 0, 0, 0, hwnd, nullptr, hInstance, nullptr);

		{
			Win32::RECT clientRc;
			Win32::GetClientRect(hwnd, &clientRc);
			int w = clientRc.right;
			int parts[] = { w / 2, w - 100, -1 };
			Win32::SendMessageW(state->statusbarHwnd, Win32::StatusBar::SetParts,
				3, reinterpret_cast<Win32::LPARAM>(parts));
			Win32::SendMessageW(state->statusbarHwnd, Win32::StatusBar::SetTextW, 2,
				reinterpret_cast<Win32::LPARAM>(L"Ready"));
		}

		state->toolboxHwnd = Win32::CreateWindowExW(
			0, L"LISTBOX", nullptr,
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::Border |
				Win32::Styles::ListBoxNotify | Win32::Styles::ListBoxNoIntegralHeight,
			0, 0, tw, state->form.height,
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
			tw, 0, state->form.width, state->form.height,
			hwnd, nullptr, hInstance, nullptr);

		Win32::SetWindowLongPtrW(state->canvasHwnd, Win32::Gwlp_UserData,
			reinterpret_cast<Win32::LONG_PTR>(state));

		state->propertyHwnd = Win32::CreateWindowExW(
			0, L"PropertyPanel", nullptr,
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::Border |
				Win32::Styles::VScroll,
			state->form.width + tw, 0,
			pw, state->form.height,
			hwnd, nullptr, hInstance, nullptr);

		Win32::SetWindowLongPtrW(state->propertyHwnd, Win32::Gwlp_UserData,
			reinterpret_cast<Win32::LONG_PTR>(state));

		CreatePropertyControls(*state);

		// Load saved theme preference.
		if (LoadThemePreference())
		{
			state->theme = DarkTheme();
			auto menu = Win32::GetMenu(hwnd);
			Win32::CheckMenuItem(menu, IDM_VIEW_DARKMODE, Win32::Menu::Checked);
			ApplyTheme(*state);
		}

		// Load recent files list.
		state->recentFiles = LoadRecentFiles();
		if (!state->currentFile.empty())
		{
			AddRecentFile(state->recentFiles, state->currentFile);
			SaveRecentFiles(state->recentFiles);
		}
		RebuildRecentFilesMenu(hwnd, state->recentFiles);

		UpdateTitle(*state);
		PopulateControls(*state);
		SyncNextGroupId(*state);
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
