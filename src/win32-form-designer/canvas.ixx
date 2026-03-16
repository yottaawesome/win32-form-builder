export module designer:canvas;
import std;
import formbuilder;
import :win32;
import :state;
import :helpers;
import :properties;

namespace Designer
{

	void RefreshZOrderPanel(DesignState& state);

	export void PopulateControls(DesignState& state)
	{
		// Flatten nested children into top-level controls so all controls
		// appear on the designer canvas (designer uses a flat model).
		auto flatControls = std::vector<FormDesigner::Control>{};
		for (auto& control : state.form.controls)
		{
			flatControls.push_back(control);
			if (!control.children.empty())
			{
				for (auto child : control.children)
				{
					child.rect.x += control.rect.x;
					child.rect.y += control.rect.y;
					flatControls.push_back(std::move(child));
				}
				// Clear children on the GroupBox — designer treats everything flat.
				flatControls[flatControls.size() - 1 - control.children.size()].children.clear();
			}
		}
		state.form.controls = std::move(flatControls);

		for (auto& control : state.form.controls)
		{
			auto* className = FormDesigner::ClassNameFor(control.type);
			if (!className)
				continue;

			auto style = Win32::DWORD{
				Win32::Styles::Child | Win32::Styles::Visible |
				FormDesigner::ImpliedStyleFor(control.type) |
				FormDesigner::AlignmentStyleFor(control.type, control.textAlign) |
				control.style};

			auto hwnd = Win32::CreateWindowExW(
				control.exStyle,
				className,
				control.text.c_str(),
				style,
				control.rect.x + RulerOffset(state),
				control.rect.y + RulerOffset(state),
				control.rect.width, control.rect.height,
				state.canvasHwnd,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(control.id)),
				state.hInstance,
				nullptr);

			if (!hwnd)
				continue;

			Win32::SendMessageW(hwnd, Win32::Messages::SetFont,
				reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);

			Win32::SetWindowSubclass(hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
			state.entries.push_back({ &control, hwnd });
		}
	}

	export void RebuildControls(DesignState& state)
	{
		for (auto& entry : state.entries)
			Win32::DestroyWindow(entry.hwnd);
		state.entries.clear();
		state.selection.clear();

		// Sync guides from form to design state.
		state.userGuides.clear();
		for (auto& g : state.form.guides)
			state.userGuides.push_back({ g.horizontal, g.position });

		PopulateControls(state);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
		UpdatePropertyPanel(state);
		RefreshZOrderPanel(state);
	}

	// Syncs user guides back into the form for serialization.
	export void SyncGuidesToForm(DesignState& state)
	{
		state.form.guides.clear();
		for (auto& g : state.userGuides)
			state.form.guides.push_back({ g.horizontal, g.position });
	}

	// Preview window procedure — mirrors FormWndProc but does NOT call PostQuitMessage.
	auto __stdcall PreviewWndProc(Win32::HWND hwnd, Win32::UINT msg,
		Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
	{
		auto* state = reinterpret_cast<DesignState*>(
			Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

		switch (msg)
		{
		case Win32::Messages::EraseBkgnd:
		{
			if (state && state->form.backgroundColor != -1)
			{
				auto hdc = reinterpret_cast<Win32::HDC>(wParam);
				auto rc = Win32::RECT{};
				Win32::GetClientRect(hwnd, &rc);
				auto brush = Win32::CreateSolidBrush(
					static_cast<Win32::DWORD>(state->form.backgroundColor));
				Win32::FillRect(hdc, &rc, brush);
				Win32::DeleteObject(brush);
				return 1;
			}
			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		case Win32::Messages::Close:
			Win32::DestroyWindow(hwnd);
			return 0;

		case Win32::Messages::NcDestroy:
			if (state)
				state->previewHwnd = nullptr;
			return 0;
		}

		return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	export void PreviewForm(DesignState& state)
	{
		// Close existing preview if open.
		if (state.previewHwnd && Win32::IsWindow(state.previewHwnd))
			Win32::DestroyWindow(state.previewHwnd);
		state.previewHwnd = nullptr;

		static bool registered = false;
		if (!registered)
		{
			Win32::WNDCLASSEXW wc = {
				.cbSize = sizeof(Win32::WNDCLASSEXW),
				.style = Win32::Cs_HRedraw | Win32::Cs_VRedraw,
				.lpfnWndProc = PreviewWndProc,
				.hInstance = state.hInstance,
				.hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
				.hbrBackground = reinterpret_cast<Win32::HBRUSH>(Win32::ColorWindow + 1),
				.lpszClassName = L"DesignerPreview",
			};
			Win32::RegisterClassExW(&wc);
			registered = true;
		}

		auto& form = state.form;
		auto style = form.style != 0 ? form.style : Win32::Styles::OverlappedWindow;

		auto rc = Win32::RECT{ 0, 0, form.width, form.height };
		Win32::AdjustWindowRectEx(&rc, style, 0, form.exStyle);

		auto title = std::wstring{L"Preview: "} + form.title;
		state.previewHwnd = Win32::CreateWindowExW(
			form.exStyle,
			L"DesignerPreview",
			title.c_str(),
			style,
			Win32::Cw_UseDefault, Win32::Cw_UseDefault,
			rc.right - rc.left, rc.bottom - rc.top,
			nullptr, nullptr, state.hInstance, nullptr);

		if (!state.previewHwnd) return;

		Win32::SetWindowLongPtrW(state.previewHwnd, Win32::Gwlp_UserData,
			reinterpret_cast<Win32::LONG_PTR>(&state));

		FormDesigner::CreateChildren(state.previewHwnd, state.hInstance, form.controls);

		Win32::ShowWindow(state.previewHwnd, Win32::Sw_ShowDefault);
		Win32::UpdateWindow(state.previewHwnd);
	}

	void PlaceControl(DesignState& state, int x, int y)
	{
		PushUndo(state);
		auto newId = NextControlId(state);
		auto& ctrl = state.form.controls.emplace_back();

		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
			state.entries[i].control = &state.form.controls[i];

		ctrl.type = state.placementType;
		ctrl.id = newId;
		ctrl.rect = { x, y, 100, 25 };

		if (state.snapToGrid)
			SnapRectToGrid(ctrl.rect, state.gridSize);

		switch (ctrl.type)
		{
		case FormDesigner::ControlType::Button:      ctrl.text = L"Button"; break;
		case FormDesigner::ControlType::CheckBox:    ctrl.text = L"CheckBox"; break;
		case FormDesigner::ControlType::RadioButton: ctrl.text = L"RadioButton"; break;
		case FormDesigner::ControlType::Label:       ctrl.text = L"Label"; break;
		case FormDesigner::ControlType::TextBox:     ctrl.text = L"TextBox"; break;
		case FormDesigner::ControlType::GroupBox:    ctrl.text = L"GroupBox"; ctrl.rect.height = 100; break;
		case FormDesigner::ControlType::ListBox:     ctrl.rect.height = 80; break;
		case FormDesigner::ControlType::ComboBox:    break;
		case FormDesigner::ControlType::ProgressBar:     ctrl.rect.height = 20; break;
		case FormDesigner::ControlType::TrackBar:        ctrl.rect.height = 30; break;
		case FormDesigner::ControlType::DateTimePicker:  break;
		case FormDesigner::ControlType::TabControl:      ctrl.rect = { x, y, 200, 150 }; break;
		case FormDesigner::ControlType::ListView:        ctrl.rect = { x, y, 200, 120 }; break;
		case FormDesigner::ControlType::TreeView:        ctrl.rect = { x, y, 200, 150 }; break;
		case FormDesigner::ControlType::UpDown:          ctrl.rect = { x, y, 20, 25 }; break;
		case FormDesigner::ControlType::RichEdit:        ctrl.rect = { x, y, 200, 100 }; break;
		default: ctrl.text = L"Control"; break;
		}

		auto* className = FormDesigner::ClassNameFor(ctrl.type);
		auto style = Win32::DWORD{
			Win32::Styles::Child | Win32::Styles::Visible |
			FormDesigner::ImpliedStyleFor(ctrl.type) |
			FormDesigner::AlignmentStyleFor(ctrl.type, ctrl.textAlign) |
			ctrl.style};

		auto hwnd = Win32::CreateWindowExW(
			ctrl.exStyle,
			className,
			ctrl.text.c_str(),
			style,
			ctrl.rect.x + RulerOffset(state),
			ctrl.rect.y + RulerOffset(state),
			ctrl.rect.width, ctrl.rect.height,
			state.canvasHwnd,
			reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(ctrl.id)),
			state.hInstance,
			nullptr);

		if (hwnd)
		{
			Win32::SendMessageW(hwnd, Win32::Messages::SetFont,
				reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);
			Win32::SetWindowSubclass(hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
			state.entries.push_back({ &ctrl, hwnd });
			state.selection = { static_cast<int>(state.entries.size()) - 1 };
		}

		state.placementMode = false;
		Win32::SendMessageW(state.toolboxHwnd, Win32::ListBox::SetCurSel,
			static_cast<Win32::WPARAM>(-1), 0);
		MarkDirty(state);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
		UpdatePropertyPanel(state);
		RefreshZOrderPanel(state);
	}

	export void CancelPlacement(DesignState& state)
	{
		if (state.placementMode)
		{
			state.placementMode = false;
			Win32::SendMessageW(state.toolboxHwnd, Win32::ListBox::SetCurSel,
				static_cast<Win32::WPARAM>(-1), 0);
		}
	}

	export void Undo(DesignState& state)
	{
		if (state.undoStack.empty()) return;
		state.redoStack.push_back(std::move(state.form));
		state.form = std::move(state.undoStack.back());
		state.undoStack.pop_back();
		state.selection.clear();
		RebuildControls(state);
		MarkDirty(state);
	}

	export void Redo(DesignState& state)
	{
		if (state.redoStack.empty()) return;
		state.undoStack.push_back(std::move(state.form));
		state.form = std::move(state.redoStack.back());
		state.redoStack.pop_back();
		state.selection.clear();
		RebuildControls(state);
		MarkDirty(state);
	}

	export void DeleteSelectedControls(DesignState& state);

	export void SelectAll(DesignState& state)
	{
		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
			state.selection.insert(i);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
		UpdatePropertyPanel(state);
	}

	export void MoveControlInZOrder(DesignState& state, int from, int to)
	{
		if (from == to) return;
		int count = static_cast<int>(state.form.controls.size());
		if (from < 0 || from >= count || to < 0 || to >= count) return;

		PushUndo(state);

		// Swap adjacent entries to move from→to, preserving relative order of others.
		int step = (to > from) ? 1 : -1;
		for (int i = from; i != to; i += step)
		{
			int next = i + step;
			std::swap(state.form.controls[i], state.form.controls[next]);
			std::swap(state.entries[i], state.entries[next]);
		}

		// Fix up control pointers.
		for (int i = 0; i < count; ++i)
			state.entries[i].control = &state.form.controls[i];

		// Update Win32 z-order to match vector order.
		Win32::HWND prevHwnd = Win32::HwndBottom;
		for (int i = 0; i < count; ++i)
		{
			Win32::SetWindowPos(state.entries[i].hwnd, prevHwnd, 0, 0, 0, 0,
				Win32::Swp::NoMove | Win32::Swp::NoSize | Win32::Swp::NoActivate);
			prevHwnd = state.entries[i].hwnd;
		}

		// Remap selection to follow the moved control.
		if (state.selection.contains(from) && !state.selection.contains(to))
		{
			state.selection.erase(from);
			state.selection.insert(to);
		}

		MarkDirty(state);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
		UpdatePropertyPanel(state);
		RefreshZOrderPanel(state);
	}

	void RefreshZOrderPanel(DesignState& state)
	{
		if (!state.zorderHwnd) return;
		auto list = Win32::GetDlgItem(state.zorderHwnd, IDC_ZORDER_LIST);
		if (!list) return;

		Win32::SendMessageW(list, Win32::ListBox::ResetContent, 0, 0);
		for (int i = 0; i < static_cast<int>(state.form.controls.size()); ++i)
		{
			auto& ctrl = state.form.controls[i];
			auto name = ControlTypeDisplayName(ctrl.type);
			auto label = std::format(L"{}: {} - {} (tab: {})", i, name,
				ctrl.text.empty() ? L"(no text)" : ctrl.text, ctrl.tabIndex);
			Win32::SendMessageW(list, Win32::ListBox::AddString, 0,
				reinterpret_cast<Win32::LPARAM>(label.c_str()));
		}
	}

	export void CopySelected(DesignState& state)
	{
		if (state.selection.empty()) return;
		state.clipboard.clear();
		for (int idx : state.selection)
		{
			if (idx >= 0 && idx < static_cast<int>(state.entries.size()))
				state.clipboard.push_back(*state.entries[idx].control);
		}
	}

	export void CutSelected(DesignState& state)
	{
		CopySelected(state);
		if (!state.clipboard.empty())
			DeleteSelectedControls(state);
	}

	export void PasteControl(DesignState& state)
	{
		if (state.clipboard.empty()) return;

		PushUndo(state);
		constexpr int PASTE_OFFSET = 20;

		state.selection.clear();

		for (auto& src : state.clipboard)
		{
			auto ctrl = src;
			ctrl.id = NextControlId(state);
			ctrl.rect.x += PASTE_OFFSET;
			ctrl.rect.y += PASTE_OFFSET;

			state.form.controls.push_back(std::move(ctrl));

			// Fix up existing entry pointers after push_back.
			for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
				state.entries[i].control = &state.form.controls[i];

			auto& placed = state.form.controls.back();
			auto* className = FormDesigner::ClassNameFor(placed.type);
			auto style = Win32::DWORD{
				Win32::Styles::Child | Win32::Styles::Visible |
				FormDesigner::ImpliedStyleFor(placed.type) |
				FormDesigner::AlignmentStyleFor(placed.type, placed.textAlign) |
				placed.style};

			auto hwnd = Win32::CreateWindowExW(
				placed.exStyle, className, placed.text.c_str(), style,
				placed.rect.x, placed.rect.y,
				placed.rect.width, placed.rect.height,
				state.canvasHwnd,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(placed.id)),
				state.hInstance, nullptr);

			if (hwnd)
			{
				Win32::SendMessageW(hwnd, Win32::Messages::SetFont,
					reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);
				Win32::SetWindowSubclass(hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
				state.entries.push_back({ &placed, hwnd });
				state.selection.insert(static_cast<int>(state.entries.size()) - 1);
			}
		}

		MarkDirty(state);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
		UpdatePropertyPanel(state);
	}

	export void DuplicateSelected(DesignState& state)
	{
		CopySelected(state);
		PasteControl(state);
	}

	void DeleteSelectedControls(DesignState& state)
	{
		if (state.selection.empty()) return;

		// Filter out locked controls.
		std::vector<int> deletable;
		for (int idx : state.selection)
			if (idx >= 0 && idx < static_cast<int>(state.entries.size()) &&
				!state.entries[idx].control->locked)
				deletable.push_back(idx);
		if (deletable.empty()) return;

		PushUndo(state);

		// Delete in reverse index order to keep earlier indices valid.
		std::sort(deletable.rbegin(), deletable.rend());

		for (int idx : deletable)
		{
			Win32::DestroyWindow(state.entries[idx].hwnd);
			state.form.controls.erase(state.form.controls.begin() + idx);
			state.entries.erase(state.entries.begin() + idx);
		}

		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
			state.entries[i].control = &state.form.controls[i];

		state.selection.clear();
		MarkDirty(state);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
		UpdatePropertyPanel(state);
		RefreshZOrderPanel(state);
	}

	export auto CanvasProc(Win32::HWND hwnd, Win32::UINT msg,
		Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
	{
		auto* state = reinterpret_cast<DesignState*>(
			Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

		switch (msg)
		{
		case Win32::Messages::EraseBkgnd:
		{
			auto hdc = reinterpret_cast<Win32::HDC>(wParam);
			Win32::RECT rc;
			Win32::GetClientRect(hwnd, &rc);
			int offset = state ? RulerOffset(*state) : 0;
			int formW = state ? state->form.width : rc.right;
			int formH = state ? state->form.height : rc.bottom;

			// Fill entire canvas with workspace color.
			auto workspaceBrush = Win32::CreateSolidBrush(state->theme.canvasBackground);
			Win32::FillRect(hdc, &rc, workspaceBrush);
			Win32::DeleteObject(workspaceBrush);

			// Fill form area with form background color (or system window color).
			Win32::RECT formArea = { offset, offset, formW + offset, formH + offset };
			if (state && state->form.backgroundColor != -1)
			{
				auto brush = Win32::CreateSolidBrush(
					static_cast<Win32::COLORREF>(state->form.backgroundColor));
				Win32::FillRect(hdc, &formArea, brush);
				Win32::DeleteObject(brush);
			}
			else
			{
				auto sysBrush = Win32::CreateSolidBrush(
					Win32::GetSysColor(Win32::ColorWindow));
				Win32::FillRect(hdc, &formArea, sysBrush);
				Win32::DeleteObject(sysBrush);
			}

			return 1;
		}

		case Win32::Messages::Paint:
		{
			Win32::PAINTSTRUCT ps;
			Win32::BeginPaint(hwnd, &ps);
			Win32::EndPaint(hwnd, &ps);

			if (state)
			{
				auto hdc = Win32::GetDC(hwnd);
				int offset = RulerOffset(*state);
				int formW = state->form.width;
				int formH = state->form.height;

				// Draw form boundary border.
				auto borderPen = Win32::CreatePen(Win32::PenStyles::Solid, 1,
					state->theme.formBorder);
				auto oldPen = Win32::SelectObject(hdc, borderPen);
				auto nullBrush = static_cast<Win32::HBRUSH>(Win32::GetStockObject(Win32::NullBrush));
				auto oldBrush = Win32::SelectObject(hdc, nullBrush);
				Win32::Rectangle(hdc, offset, offset, formW + offset + 1, formH + offset + 1);
				Win32::SelectObject(hdc, oldBrush);
				Win32::SelectObject(hdc, oldPen);
				Win32::DeleteObject(borderPen);

				if (state->showGrid)
				{
					auto dotColor = static_cast<Win32::COLORREF>(state->theme.gridDot);
					for (int gx = 0; gx < formW; gx += state->gridSize)
						for (int gy = 0; gy < formH; gy += state->gridSize)
							Win32::SetPixel(hdc, gx + offset, gy + offset, dotColor);
				}

				DrawUserGuides(*state, hdc);
				DrawSelection(*state, hdc);
				DrawAlignGuides(*state, hdc);
				DrawRulers(*state, hdc);
				if (state->tabOrderMode)
					DrawTabOrderBadges(*state, hdc);
				if (state->lastCursorPos.x >= 0)
					DrawRulerCursorIndicator(*state, hdc,
						state->lastCursorPos.x, state->lastCursorPos.y);
				Win32::ReleaseDC(hwnd, hdc);
			}
			return 0;
		}

		case Win32::Messages::LButtonDown:
		{
			if (!state) break;
			Win32::SetFocus(hwnd);
			int rawX = Win32::GetXParam(lParam);
			int rawY = Win32::GetYParam(lParam);
			int offset = RulerOffset(*state);
			int x = rawX - offset;  // form coordinates
			int y = rawY - offset;

			// Check if click is in the ruler area — start guide drag.
			if (state->showRulers && (rawX < RULER_SIZE || rawY < RULER_SIZE))
			{
				if (rawX < RULER_SIZE && rawY >= RULER_SIZE)
				{
					// Left ruler: create vertical guide.
					state->dragMode = DragMode::CreateGuide;
					state->draggingGuideHorizontal = false;
					state->draggingGuidePos = x;
					Win32::SetCapture(hwnd);
				}
				else if (rawY < RULER_SIZE && rawX >= RULER_SIZE)
				{
					// Top ruler: create horizontal guide.
					state->dragMode = DragMode::CreateGuide;
					state->draggingGuideHorizontal = true;
					state->draggingGuidePos = y;
					Win32::SetCapture(hwnd);
				}
				return 0;
			}

			if (state->placementMode)
			{
				PlaceControl(*state, x, y);
				return 0;
			}

			// Tab order mode: click assigns next tab index.
			if (state->tabOrderMode)
			{
				int hit = HitTest(*state, x, y);
				if (hit >= 0)
				{
					state->entries[hit].control->tabIndex = state->tabOrderNext;
					state->tabOrderNext++;
					MarkDirty(*state);
					Win32::InvalidateRect(hwnd, nullptr, true);
					UpdatePropertyPanel(*state);
				}
				return 0;
			}

			// Check if click is near the form boundary edges for form resize.
			auto edge = HitTestFormBoundary(*state, x, y);
			if (edge != FormEdge::None && !state->placementMode)
			{
				PushUndo(*state);
				state->dragMode = DragMode::ResizeForm;
				state->formEdge = edge;
				state->dragStart = { x, y };
				state->controlStartSize = { state->form.width, state->form.height };
				Win32::SetCapture(hwnd);
				return 0;
			}

			bool ctrlHeld = (Win32::GetKeyState(Win32::Keys::Control) & 0x8000) != 0;

			// Resize handles: only when exactly one unlocked control is selected.
			int handle = HitTestHandle(*state, x, y);
			if (handle >= 0)
			{
				int sel = SingleSelection(*state);
				if (state->entries[sel].control->locked) return 0;
				PushUndo(*state);
				auto& r = state->entries[sel].control->rect;
				state->dragMode = DragMode::Resize;
				state->activeHandle = handle;
				state->dragAnchor = sel;
				state->dragStart = { x, y };
				state->controlStart = { r.x, r.y };
				state->controlStartSize = { r.width, r.height };
				Win32::SetCapture(hwnd);
				return 0;
			}

			int hit = HitTest(*state, x, y);

			if (hit >= 0)
			{
				if (ctrlHeld)
				{
					// Ctrl+Click: toggle selection.
					if (IsSelected(*state, hit))
						state->selection.erase(hit);
					else
						state->selection.insert(hit);
				}
				else if (!IsSelected(*state, hit))
				{
					// Click on unselected without Ctrl: select this control
					// (and all group members if grouped).
					auto gid = state->entries[hit].control->groupId;
					if (gid != 0)
					{
						state->selection.clear();
						for (int i = 0; i < static_cast<int>(state->entries.size()); ++i)
							if (state->entries[i].control->groupId == gid)
								state->selection.insert(i);
					}
					else
					{
						state->selection = { hit };
					}
				}
				// Click on already-selected without Ctrl: keep current selection (for drag).

				Win32::InvalidateRect(hwnd, nullptr, true);
				UpdatePropertyPanel(*state);

				// Start dragging all selected controls (skip if all are locked).
				if (IsSelected(*state, hit))
				{
					bool allLocked = true;
					for (int idx : state->selection)
						if (!state->entries[idx].control->locked) { allLocked = false; break; }
					if (allLocked) return 0;

					PushUndo(*state);
					state->dragMode = DragMode::Move;
					state->activeHandle = -1;
					state->dragAnchor = hit;
					state->dragStart = { x, y };
					state->dragOrigins.clear();
					for (int idx : state->selection)
						state->dragOrigins[idx] = { state->entries[idx].control->rect.x,
													state->entries[idx].control->rect.y };
					Win32::SetCapture(hwnd);
				}
			}
			else
			{
				// Click on empty space: deselect all.
				if (!ctrlHeld)
					state->selection.clear();
				Win32::InvalidateRect(hwnd, nullptr, true);
				UpdatePropertyPanel(*state);
			}
			return 0;
		}

		case Win32::Messages::MouseMove:
		{
			if (!state) break;
			int rawX = Win32::GetXParam(lParam);
			int rawY = Win32::GetYParam(lParam);
			int offset = RulerOffset(*state);
			int x = rawX - offset;  // form coordinates
			int y = rawY - offset;
			state->lastCursorPos = { x, y };

			if (state->dragMode == DragMode::CreateGuide)
			{
				state->draggingGuidePos = state->draggingGuideHorizontal ? y : x;
				Win32::InvalidateRect(hwnd, nullptr, true);
				return 0;
			}

			if (state->dragMode == DragMode::ResizeForm)
			{
				int dx = x - state->dragStart.x;
				int dy = y - state->dragStart.y;
				constexpr int MIN_FORM_SIZE = 50;

				if (state->formEdge == FormEdge::Right || state->formEdge == FormEdge::BottomRight)
				{
					int newW = state->controlStartSize.cx + dx;
					state->form.width = std::max(newW, MIN_FORM_SIZE);
				}
				if (state->formEdge == FormEdge::Bottom || state->formEdge == FormEdge::BottomRight)
				{
					int newH = state->controlStartSize.cy + dy;
					state->form.height = std::max(newH, MIN_FORM_SIZE);
				}

				if (state->snapToGrid)
				{
					state->form.width = SnapValue(state->form.width, state->gridSize);
					state->form.height = SnapValue(state->form.height, state->gridSize);
				}

				Win32::InvalidateRect(hwnd, nullptr, true);
				MarkDirty(*state);
				UpdatePropertyPanel(*state);
				return 0;
			}

			if (state->dragMode == DragMode::Move)
			{
				int dx = x - state->dragStart.x;
				int dy = y - state->dragStart.y;

				// Move all selected unlocked controls by the same delta.
				for (int idx : state->selection)
				{
					if (idx < 0 || idx >= static_cast<int>(state->entries.size())) continue;
					if (state->entries[idx].control->locked) continue;
					auto it = state->dragOrigins.find(idx);
					if (it == state->dragOrigins.end()) continue;
					auto& entry = state->entries[idx];
					entry.control->rect.x = it->second.x + dx;
					entry.control->rect.y = it->second.y + dy;
				}

				// Snap guides based on the drag anchor control.
				if (state->dragAnchor >= 0 && state->dragAnchor < static_cast<int>(state->entries.size()))
				{
					auto& anchorRect = state->entries[state->dragAnchor].control->rect;
					auto prevX = anchorRect.x;
					auto prevY = anchorRect.y;
					FindAlignGuides(*state, anchorRect);
					int snapDx = anchorRect.x - prevX;
					int snapDy = anchorRect.y - prevY;

					// Grid snap if no align guide fired on that axis.
					if (state->snapToGrid)
					{
						if (snapDx == 0)
						{
							int gridX = SnapValue(anchorRect.x, state->gridSize);
							snapDx = gridX - anchorRect.x;
							anchorRect.x = gridX;
						}
						if (snapDy == 0)
						{
							int gridY = SnapValue(anchorRect.y, state->gridSize);
							snapDy = gridY - anchorRect.y;
							anchorRect.y = gridY;
						}
					}

					// Apply snap adjustment to all other selected controls.
					if (snapDx != 0 || snapDy != 0)
					{
						for (int idx : state->selection)
						{
							if (idx < 0 || idx >= static_cast<int>(state->entries.size())) continue;
							if (idx == state->dragAnchor) continue;
							state->entries[idx].control->rect.x += snapDx;
							state->entries[idx].control->rect.y += snapDy;
						}
					}
				}

				for (int idx : state->selection)
				{
					if (idx < 0 || idx >= static_cast<int>(state->entries.size())) continue;
					auto& entry = state->entries[idx];
					Win32::MoveWindow(entry.hwnd,
						entry.control->rect.x + offset,
						entry.control->rect.y + offset,
						entry.control->rect.width, entry.control->rect.height,
						true);
				}

				Win32::InvalidateRect(hwnd, nullptr, true);
				MarkDirty(*state);
				UpdatePropertyPanel(*state);
				return 0;
			}

			if (state->dragMode == DragMode::Resize)
			{
				int sel = SingleSelection(*state);
				if (sel < 0) break;
				auto& entry = state->entries[sel];
				int dx = x - state->dragStart.x;
				int dy = y - state->dragStart.y;

				ApplyResize(entry.control->rect, state->activeHandle, dx, dy,
					state->controlStart, state->controlStartSize);

				if (state->snapToGrid)
					SnapRectToGrid(entry.control->rect, state->gridSize);

				Win32::MoveWindow(entry.hwnd,
					entry.control->rect.x + offset,
					entry.control->rect.y + offset,
					entry.control->rect.width, entry.control->rect.height,
					true);
				Win32::InvalidateRect(hwnd, nullptr, true);
				MarkDirty(*state);
				UpdatePropertyPanel(*state);
				return 0;
			}

			if (state->placementMode)
			{
				Win32::SetCursor(Win32::LoadCursorW(nullptr, Win32::Cursors::Cross));
			}
			else
			{
				int handle = HitTestHandle(*state, x, y);
				if (handle >= 0)
					Win32::SetCursor(Win32::LoadCursorW(nullptr, CursorForHandle(handle)));
				else
				{
					auto edge = HitTestFormBoundary(*state, x, y);
					if (edge != FormEdge::None)
						Win32::SetCursor(Win32::LoadCursorW(nullptr, CursorForFormEdge(edge)));
					else
						Win32::SetCursor(Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow));
				}
			}

			// Update cursor position in status bar.
			if (state->statusbarHwnd)
			{
				auto pos = std::format(L"X: {}, Y: {}", x, y);
				Win32::SendMessageW(state->statusbarHwnd, Win32::StatusBar::SetTextW, 1,
					reinterpret_cast<Win32::LPARAM>(pos.c_str()));
			}

			// Update ruler cursor indicator — invalidate ruler strips only.
			if (state->showRulers)
			{
				Win32::RECT topRuler = { 0, 0, 32767, RULER_SIZE };
				Win32::RECT leftRuler = { 0, 0, RULER_SIZE, 32767 };
				Win32::InvalidateRect(hwnd, &topRuler, false);
				Win32::InvalidateRect(hwnd, &leftRuler, false);
			}
			break;
		}

		case Win32::Messages::LButtonUp:
		{
			if (!state || state->dragMode == DragMode::None) break;

			if (state->dragMode == DragMode::CreateGuide)
			{
				int offset = RulerOffset(*state);
				int rawX = Win32::GetXParam(lParam);
				int rawY = Win32::GetYParam(lParam);
				int pos = state->draggingGuidePos;

				// Only commit if dragged into the canvas area (past the ruler).
				bool inCanvas = state->draggingGuideHorizontal
					? (rawY >= RULER_SIZE)
					: (rawX >= RULER_SIZE);
				if (inCanvas && pos >= 0)
					state->userGuides.push_back({ state->draggingGuideHorizontal, pos });

				state->draggingGuidePos = -1;
			}

			state->dragMode = DragMode::None;
			state->formEdge = FormEdge::None;
			state->activeHandle = -1;
			state->guides.clear();
			Win32::ReleaseCapture();
			Win32::InvalidateRect(hwnd, nullptr, true);
			return 0;
		}

		case Win32::Messages::SetCursorMsg:
		{
			if (state && Win32::GetLowWord(static_cast<Win32::WPARAM>(lParam)) == Win32::HitTestValues::Client)
				return 1;
			break;
		}

		case Win32::Messages::RButtonUp:
		{
			if (!state) break;
			int offset = RulerOffset(*state);
			int x = Win32::GetXParam(lParam) - offset;  // form coordinates
			int y = Win32::GetYParam(lParam) - offset;

			// Check if right-clicking near a user guide to delete it.
			int guideHit = HitTestUserGuide(*state, x, y);
			if (guideHit >= 0)
			{
				auto menu = Win32::CreatePopupMenu();
				Win32::AppendMenuW(menu, Win32::Menu::String, 1, L"&Delete Guide");
				Win32::POINT screen = { x + offset, y + offset };
				Win32::ClientToScreen(hwnd, &screen);
				auto cmd = Win32::TrackPopupMenu(menu,
					Win32::TrackPopup::LeftAlign | Win32::TrackPopup::TopAlign |
					Win32::TrackPopup::ReturnCmd | Win32::TrackPopup::RightButton,
					screen.x, screen.y, 0, hwnd, nullptr);
				Win32::DestroyMenu(menu);
				if (cmd == 1)
				{
					state->userGuides.erase(state->userGuides.begin() + guideHit);
					Win32::InvalidateRect(hwnd, nullptr, true);
				}
				return 0;
			}

			// If right-clicked on a control that isn't selected, select it.
			int hit = HitTest(*state, x, y);
			if (hit >= 0 && !IsSelected(*state, hit))
			{
				state->selection = { hit };
				Win32::InvalidateRect(hwnd, nullptr, true);
				UpdatePropertyPanel(*state);
			}
			else if (hit < 0)
			{
				state->selection.clear();
				Win32::InvalidateRect(hwnd, nullptr, true);
				UpdatePropertyPanel(*state);
			}

			auto menu = Win32::CreatePopupMenu();
			bool hasSelection = !state->selection.empty();
			bool hasClipboard = !state->clipboard.empty();

			if (hasSelection)
			{
				// Determine lock label: "Lock" if any unlocked, "Unlock" if all locked.
				bool allLocked = true;
				for (int idx : state->selection)
					if (!state->entries[idx].control->locked) { allLocked = false; break; }

				Win32::AppendMenuW(menu, Win32::Menu::String, IDM_EDIT_CUT, L"Cu&t\tCtrl+X");
				Win32::AppendMenuW(menu, Win32::Menu::String, IDM_EDIT_COPY, L"&Copy\tCtrl+C");
				Win32::AppendMenuW(menu, Win32::Menu::String, IDM_EDIT_PASTE, L"&Paste\tCtrl+V");
				Win32::AppendMenuW(menu, Win32::Menu::String, IDM_EDIT_DUPLICATE, L"&Duplicate\tCtrl+D");
				Win32::AppendMenuW(menu, Win32::Menu::Separator, 0, nullptr);
				Win32::AppendMenuW(menu, Win32::Menu::String, IDM_EDIT_DELETE, L"&Delete\tDel");
				Win32::AppendMenuW(menu, Win32::Menu::Separator, 0, nullptr);
				Win32::AppendMenuW(menu, Win32::Menu::String, IDM_CTX_LOCK,
					allLocked ? L"&Unlock" : L"&Lock");
				Win32::AppendMenuW(menu, Win32::Menu::Separator, 0, nullptr);
				Win32::AppendMenuW(menu, Win32::Menu::String, IDM_CTX_TOFRONT, L"Bring to &Front");
				Win32::AppendMenuW(menu, Win32::Menu::String, IDM_CTX_TOBACK, L"Send to &Back");
				Win32::AppendMenuW(menu, Win32::Menu::Separator, 0, nullptr);
				Win32::AppendMenuW(menu,
					state->selection.size() >= 2 ? Win32::Menu::String : Win32::Menu::Grayed,
					IDM_EDIT_GROUP, L"&Group\tCtrl+G");
				// Show Ungroup only if any selected control is grouped.
				bool anyGrouped = false;
				for (int idx : state->selection)
					if (state->entries[idx].control->groupId != 0) { anyGrouped = true; break; }
				Win32::AppendMenuW(menu,
					anyGrouped ? Win32::Menu::String : Win32::Menu::Grayed,
					IDM_EDIT_UNGROUP, L"U&ngroup\tCtrl+Shift+G");
			}
			else
			{
				Win32::AppendMenuW(menu, hasClipboard ? Win32::Menu::String : Win32::Menu::Grayed,
					IDM_EDIT_PASTE, L"&Paste\tCtrl+V");
				Win32::AppendMenuW(menu, Win32::Menu::Separator, 0, nullptr);
				Win32::AppendMenuW(menu,
					state->entries.empty() ? Win32::Menu::Grayed : Win32::Menu::String,
					IDM_EDIT_SELECTALL, L"Select &All\tCtrl+A");
			}

			Win32::POINT screen = { x + offset, y + offset };
			Win32::ClientToScreen(hwnd, &screen);

			auto cmd = Win32::TrackPopupMenu(menu,
				Win32::TrackPopup::LeftAlign | Win32::TrackPopup::TopAlign |
				Win32::TrackPopup::ReturnCmd | Win32::TrackPopup::RightButton,
				screen.x, screen.y, 0, hwnd, nullptr);

			Win32::DestroyMenu(menu);

			if (cmd)
			{
				switch (cmd)
				{
				case IDM_EDIT_CUT:       CutSelected(*state); break;
				case IDM_EDIT_COPY:      CopySelected(*state); break;
				case IDM_EDIT_PASTE:     PasteControl(*state); break;
				case IDM_EDIT_DUPLICATE: DuplicateSelected(*state); break;
				case IDM_EDIT_DELETE:    DeleteSelectedControls(*state); break;
				case IDM_EDIT_SELECTALL: SelectAll(*state); break;
				case IDM_CTX_TOFRONT:
				{
					// Move all selected controls to front (index 0).
					std::vector<int> sorted(state->selection.begin(), state->selection.end());
					std::sort(sorted.begin(), sorted.end());
					for (int i = 0; i < static_cast<int>(sorted.size()); ++i)
						MoveControlInZOrder(*state, sorted[i], i);
					break;
				}
				case IDM_CTX_TOBACK:
				{
					int count = static_cast<int>(state->entries.size());
					std::vector<int> sorted(state->selection.begin(), state->selection.end());
					std::sort(sorted.rbegin(), sorted.rend());
					for (int i = 0; i < static_cast<int>(sorted.size()); ++i)
						MoveControlInZOrder(*state, sorted[i], count - 1 - i);
					break;
				}
				case IDM_CTX_LOCK:
				{
					bool allLocked = true;
					for (int idx : state->selection)
						if (!state->entries[idx].control->locked) { allLocked = false; break; }
					PushUndo(*state);
					for (int idx : state->selection)
						state->entries[idx].control->locked = !allLocked;
					MarkDirty(*state);
					Win32::InvalidateRect(hwnd, nullptr, true);
					UpdatePropertyPanel(*state);
					break;
				}
				case IDM_EDIT_GROUP:   GroupSelected(*state); break;
				case IDM_EDIT_UNGROUP: UngroupSelected(*state); break;
				}
			}
			return 0;
		}

		case Win32::Messages::KeyDown:
		{
			if (!state) break;

			// Escape exits tab order mode.
			if (state->tabOrderMode && wParam == Win32::Keys::Escape)
			{
				state->tabOrderMode = false;
				state->tabOrderNext = 1;
				auto menu = Win32::GetMenu(state->surfaceHwnd);
				Win32::CheckMenuItem(menu, IDM_VIEW_TABORDER, Win32::Menu::Unchecked);
				Win32::InvalidateRect(hwnd, nullptr, true);
				return 0;
			}

			if (wParam == Win32::Keys::Delete)
			{
				DeleteSelectedControls(*state);
				return 0;
			}
			if (wParam == 'A' && (Win32::GetKeyState(Win32::Keys::Control) & 0x8000))
			{
				SelectAll(*state);
				return 0;
			}

			// Arrow key nudge.
			if (wParam == Win32::Keys::Left || wParam == Win32::Keys::Right ||
				wParam == Win32::Keys::Up || wParam == Win32::Keys::Down)
			{
				if (state->selection.empty()) break;

				bool shift = (Win32::GetKeyState(Win32::Keys::Shift) & 0x8000) != 0;
				int step = shift ? 10 : (state->snapToGrid ? state->gridSize : 1);
				int dx = 0, dy = 0;

				if (wParam == Win32::Keys::Left)  dx = -step;
				if (wParam == Win32::Keys::Right) dx =  step;
				if (wParam == Win32::Keys::Up)    dy = -step;
				if (wParam == Win32::Keys::Down)  dy =  step;

				PushUndo(*state);
				int offset = RulerOffset(*state);
				for (int idx : state->selection)
				{
					if (state->entries[idx].control->locked) continue;
					auto& r = state->entries[idx].control->rect;
					r.x += dx;
					r.y += dy;
					if (state->snapToGrid && !shift)
						SnapRectToGrid(r, state->gridSize);
					Win32::MoveWindow(state->entries[idx].hwnd,
						r.x + offset, r.y + offset, r.width, r.height, true);
				}
				Win32::InvalidateRect(hwnd, nullptr, true);
				MarkDirty(*state);
				UpdatePropertyPanel(*state);
				return 0;
			}
			break;
		}
		}

		return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
	}

}
