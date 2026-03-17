export module designer:zorder;
import std;
import formbuilder;
import :state;
import :helpers;
import :properties;
import :canvas;

namespace Designer
{

	constexpr int BASE_ZORDER_WIDTH  = 280;
	constexpr int BASE_ZORDER_HEIGHT = 380;
	constexpr int BASE_ZORDER_BTN_H  = 28;
	constexpr int BASE_ZORDER_BTN_W  = 82;
	constexpr int BASE_ZORDER_PAD    = 6;
	constexpr int BASE_ZORDER_MIN_W  = 240;
	constexpr int BASE_ZORDER_MIN_H  = 260;

	// Forward declarations for subclass procs.
	auto ZOrderEditSubclassProc(Win32::HWND, Win32::UINT, Win32::WPARAM, Win32::LPARAM,
		Win32::UINT_PTR, Win32::DWORD_PTR) -> Win32::LRESULT;
	auto ZOrderListSubclassProc(Win32::HWND, Win32::UINT, Win32::WPARAM, Win32::LPARAM,
		Win32::UINT_PTR, Win32::DWORD_PTR) -> Win32::LRESULT;

	// Returns selected indices from the multi-select ListBox.
	auto GetListBoxSelections(Win32::HWND list) -> std::vector<int>
	{
		int count = static_cast<int>(Win32::SendMessageW(list, Win32::ListBox::GetSelCount, 0, 0));
		if (count <= 0) return {};
		std::vector<int> buf(count);
		Win32::SendMessageW(list, Win32::ListBox::GetSelItems,
			static_cast<Win32::WPARAM>(count),
			reinterpret_cast<Win32::LPARAM>(buf.data()));
		return buf;
	}

	// Begins inline rename of the focused ListBox item.
	void BeginZOrderRename(DesignState& state, Win32::HWND panel)
	{
		auto list = Win32::GetDlgItem(panel, IDC_ZORDER_LIST);
		if (!list) return;
		auto sels = GetListBoxSelections(list);
		if (sels.size() != 1) return;
		int sel = sels[0];
		if (sel < 0 || sel >= static_cast<int>(state.entries.size())) return;

		Win32::RECT itemRc;
		Win32::SendMessageW(list, Win32::ListBox::GetItemRect,
			static_cast<Win32::WPARAM>(sel),
			reinterpret_cast<Win32::LPARAM>(&itemRc));

		// Map ListBox-relative rect to panel coordinates.
		Win32::MapWindowPoints(list, panel, reinterpret_cast<Win32::POINT*>(&itemRc), 2);

		auto font = reinterpret_cast<Win32::WPARAM>(
			Win32::GetStockObject(Win32::DefaultGuiFont));

		auto edit = Win32::CreateWindowExW(
			Win32::ExStyles::ClientEdge, L"EDIT",
			state.entries[sel].control->text.c_str(),
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::EditAutoHScroll,
			itemRc.left, itemRc.top,
			itemRc.right - itemRc.left, itemRc.bottom - itemRc.top,
			panel,
			reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_ZORDER_EDIT)),
			state.hInstance, nullptr);
		Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
		Win32::SendMessageW(edit, Win32::EditControl::SetSel, 0, -1);
		Win32::SetWindowSubclass(edit, ZOrderEditSubclassProc, 1,
			reinterpret_cast<Win32::DWORD_PTR>(&state));
		Win32::SetFocus(edit);
	}

	// Commits inline rename.
	void CommitZOrderRename(DesignState& state, Win32::HWND panel)
	{
		auto edit = Win32::GetDlgItem(panel, IDC_ZORDER_EDIT);
		if (!edit) return;

		auto list = Win32::GetDlgItem(panel, IDC_ZORDER_LIST);
		auto sels = GetListBoxSelections(list);
		if (sels.size() == 1)
		{
			int sel = sels[0];
			if (sel >= 0 && sel < static_cast<int>(state.entries.size()))
			{
				wchar_t buf[256] = {};
				Win32::GetWindowTextW(edit, buf, 256);
				PushUndo(state);
				state.entries[sel].control->text = buf;
				Win32::SetWindowTextW(state.entries[sel].hwnd, buf);
				MarkDirty(state);
				Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
				UpdatePropertyPanel(state);
			}
		}
		Win32::DestroyWindow(edit);
		RefreshZOrderPanel(state);
	}

	// Cancels inline rename.
	void CancelZOrderRename(Win32::HWND panel)
	{
		auto edit = Win32::GetDlgItem(panel, IDC_ZORDER_EDIT);
		if (edit) Win32::DestroyWindow(edit);
	}

	// Repositions Z-Order panel child controls after resize.
	void LayoutZOrderPanel(Win32::HWND hwnd, const DpiInfo& d)
	{
		Win32::RECT rc;
		Win32::GetClientRect(hwnd, &rc);
		int cw = rc.right;
		int ch = rc.bottom;

		int zbh  = d.Scale(BASE_ZORDER_BTN_H);
		int zbw  = d.Scale(BASE_ZORDER_BTN_W);
		int zpad = d.Scale(BASE_ZORDER_PAD);

		int buttonRows = 2;
		int btnAreaH = buttonRows * (zbh + zpad);
		int listH = ch - btnAreaH - zpad * 2;
		if (listH < 30) listH = 30;

		auto list = Win32::GetDlgItem(hwnd, IDC_ZORDER_LIST);
		if (list) Win32::MoveWindow(list, zpad, zpad, cw - zpad * 2, listH, true);

		int btnY = zpad + listH + zpad;
		int col3w = (cw - zpad * 4) / 3;

		struct BtnPos { Win32::UINT id; int col; int row; };
		BtnPos positions[] = {
			{ IDC_ZORDER_UP,     0, 0 },
			{ IDC_ZORDER_DOWN,   1, 0 },
			{ IDC_ZORDER_DELETE, 2, 0 },
			{ IDC_ZORDER_TOP,    0, 1 },
			{ IDC_ZORDER_BOTTOM, 1, 1 },
		};
		for (auto& bp : positions)
		{
			auto btn = Win32::GetDlgItem(hwnd, bp.id);
			if (btn) Win32::MoveWindow(btn, zpad + bp.col * (col3w + zpad),
				btnY + bp.row * (zbh + zpad), col3w, zbh, true);
		}
	}

	export auto ZOrderPanelProc(Win32::HWND hwnd, Win32::UINT msg,
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

			// ListBox selection changed → sync to canvas.
			if (id == IDC_ZORDER_LIST && code == Win32::Notifications::ListBoxSelChange)
			{
				CancelZOrderRename(hwnd);
				auto list = Win32::GetDlgItem(hwnd, IDC_ZORDER_LIST);
				auto sels = GetListBoxSelections(list);
				state->selection.clear();
				for (int s : sels)
					if (s >= 0 && s < static_cast<int>(state->entries.size()))
						state->selection.insert(s);
				Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
				UpdatePropertyPanel(*state);
				return 0;
			}

			// Double-click → inline rename.
			if (id == IDC_ZORDER_LIST && code == Win32::Notifications::ListBoxDoubleClick)
			{
				BeginZOrderRename(*state, hwnd);
				return 0;
			}

			// Inline edit kill focus → commit.
			if (id == IDC_ZORDER_EDIT && code == Win32::Notifications::EditKillFocus)
			{
				CommitZOrderRename(*state, hwnd);
				return 0;
			}

			// Button clicks.
			if (code == Win32::Notifications::ButtonClicked)
			{
				if (id == IDC_ZORDER_DELETE)
				{
					auto list = Win32::GetDlgItem(hwnd, IDC_ZORDER_LIST);
					auto sels = GetListBoxSelections(list);
					if (!sels.empty())
					{
						state->selection.clear();
						for (int s : sels) state->selection.insert(s);
						DeleteSelectedControls(*state);
						Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
						UpdatePropertyPanel(*state);
					}
					return 0;
				}

				auto list = Win32::GetDlgItem(hwnd, IDC_ZORDER_LIST);
				auto sels = GetListBoxSelections(list);
				if (sels.size() != 1) return 0;
				int sel = sels[0];

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
					// Re-select in multi-sel listbox.
					Win32::SendMessageW(list, Win32::ListBox::SetSel, false, -1);
					Win32::SendMessageW(list, Win32::ListBox::SetSel, true,
						static_cast<Win32::LPARAM>(newSel));
					state->selection = { newSel };
					Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
					UpdatePropertyPanel(*state);
				}
				return 0;
			}
			break;
		}

		case Win32::Messages::Size:
			if (state) LayoutZOrderPanel(hwnd, state->dpiInfo);
			return 0;

		case Win32::Messages::GetMinMaxInfo:
		{
			auto* mmi = reinterpret_cast<Win32::MINMAXINFO*>(lParam);
			if (state)
			{
				mmi->ptMinTrackSize.x = state->dpiInfo.Scale(BASE_ZORDER_MIN_W);
				mmi->ptMinTrackSize.y = state->dpiInfo.Scale(BASE_ZORDER_MIN_H);
			}
			return 0;
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

	// Subclass proc for the inline rename edit control.
	auto ZOrderEditSubclassProc(Win32::HWND hwnd, Win32::UINT msg,
		Win32::WPARAM wParam, Win32::LPARAM lParam,
		Win32::UINT_PTR, Win32::DWORD_PTR refData) -> Win32::LRESULT
	{
		auto* state = reinterpret_cast<DesignState*>(refData);
		if (msg == Win32::Messages::KeyDown)
		{
			if (wParam == Win32::Keys::Return)
			{
				auto panel = Win32::GetParent(hwnd);
				CommitZOrderRename(*state, panel);
				return 0;
			}
			if (wParam == Win32::Keys::Escape)
			{
				auto panel = Win32::GetParent(hwnd);
				CancelZOrderRename(panel);
				return 0;
			}
		}
		return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
	}

	// Subclass proc for the Z-Order ListBox to handle keyboard shortcuts.
	auto ZOrderListSubclassProc(Win32::HWND hwnd, Win32::UINT msg,
		Win32::WPARAM wParam, Win32::LPARAM lParam,
		Win32::UINT_PTR, Win32::DWORD_PTR refData) -> Win32::LRESULT
	{
		auto* state = reinterpret_cast<DesignState*>(refData);
		if (msg == Win32::Messages::KeyDown)
		{
			auto vk = static_cast<int>(wParam);
			auto panel = Win32::GetParent(hwnd);

			if (vk == Win32::Keys::F2)
			{
				BeginZOrderRename(*state, panel);
				return 0;
			}

			if (vk == Win32::Keys::Delete)
			{
				auto sels = GetListBoxSelections(hwnd);
				if (!sels.empty())
				{
					state->selection.clear();
					for (int s : sels) state->selection.insert(s);
					DeleteSelectedControls(*state);
					Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
					UpdatePropertyPanel(*state);
				}
				return 0;
			}

			// Ctrl+Up/Down → move in z-order.
			if ((vk == Win32::Keys::Up || vk == Win32::Keys::Down)
				&& (Win32::GetKeyState(Win32::Keys::Control) & 0x8000))
			{
				auto sels = GetListBoxSelections(hwnd);
				if (sels.size() != 1) return 0;
				int sel = sels[0];
				int count = static_cast<int>(state->form.controls.size());
				int newSel = (vk == Win32::Keys::Up && sel > 0) ? sel - 1
					: (vk == Win32::Keys::Down && sel < count - 1) ? sel + 1
					: sel;
				if (newSel != sel)
				{
					MoveControlInZOrder(*state, sel, newSel);
					Win32::SendMessageW(hwnd, Win32::ListBox::SetSel, false, -1);
					Win32::SendMessageW(hwnd, Win32::ListBox::SetSel, true,
						static_cast<Win32::LPARAM>(newSel));
					state->selection = { newSel };
					Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
					UpdatePropertyPanel(*state);
				}
				return 0;
			}
		}
		return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
	}

	export void ShowZOrderPanel(DesignState& state)
	{
		if (state.zorderHwnd)
		{
			Win32::SetFocus(state.zorderHwnd);
			return;
		}

		Win32::RECT rc;
		Win32::GetWindowRect(state.surfaceHwnd, &rc);

		auto& d = state.dpiInfo;
		int zw   = d.Scale(BASE_ZORDER_WIDTH);
		int zh   = d.Scale(BASE_ZORDER_HEIGHT);
		int zbh  = d.Scale(BASE_ZORDER_BTN_H);
		int zpad = d.Scale(BASE_ZORDER_PAD);

		state.zorderHwnd = Win32::CreateWindowExW(
			0, L"ZOrderPanel", L"Controls — Z-Order",
			Win32::Styles::OverlappedWindow & ~Win32::Styles::MaximizeBox,
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

		int buttonRows = 2;
		int btnAreaH = buttonRows * (zbh + zpad);
		int listH = ch - btnAreaH - zpad * 2;

		// Multi-select ListBox.
		auto list = Win32::CreateWindowExW(
			Win32::ExStyles::ClientEdge, L"LISTBOX", nullptr,
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::Border |
				Win32::Styles::ListBoxNotify | Win32::Styles::ListBoxNoIntegralHeight |
				Win32::Styles::ListBoxExtendedSel | Win32::Styles::VScroll,
			zpad, zpad, cw - zpad * 2, listH,
			state.zorderHwnd,
			reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_ZORDER_LIST)),
			state.hInstance, nullptr);
		Win32::SendMessageW(list, Win32::Messages::SetFont, font, true);
		Win32::SetWindowSubclass(list, ZOrderListSubclassProc, 1,
			reinterpret_cast<Win32::DWORD_PTR>(&state));

		int btnY = zpad + listH + zpad;
		int col3w = (cw - zpad * 4) / 3;

		struct Btn { const wchar_t* text; Win32::UINT id; int col; int row; };
		Btn buttons[] = {
			{ L"\u25B2 Up",       IDC_ZORDER_UP,     0, 0 },
			{ L"\u25BC Down",     IDC_ZORDER_DOWN,   1, 0 },
			{ L"\u2716 Delete",   IDC_ZORDER_DELETE, 2, 0 },
			{ L"\u25B2\u25B2 Top",    IDC_ZORDER_TOP,    0, 1 },
			{ L"\u25BC\u25BC Bottom", IDC_ZORDER_BOTTOM, 1, 1 },
		};

		for (auto& btn : buttons)
		{
			auto hwnd = Win32::CreateWindowExW(0, L"BUTTON", btn.text,
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
				zpad + btn.col * (col3w + zpad), btnY + btn.row * (zbh + zpad),
				col3w, zbh,
				state.zorderHwnd,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(btn.id)),
				state.hInstance, nullptr);
			Win32::SendMessageW(hwnd, Win32::Messages::SetFont, font, true);
		}

		RefreshZOrderPanel(state);
		Win32::ShowWindow(state.zorderHwnd, Win32::Sw_ShowDefault);
	}

}
