export module designer:helpers;
import std;
import formbuilder;
import :win32;
import :state;

namespace Designer
{

	auto ControlTypeDisplayName(FormDesigner::ControlType type) -> const wchar_t*;

	// Safely narrows a wide string to a std::string.
	// Assumes ASCII content (valid for C++ identifiers like event handler names).
	export auto NarrowString(const wchar_t* wstr, std::size_t len) -> std::string
	{
		auto result = std::string{};
		result.reserve(len);
		for (std::size_t i = 0; i < len; ++i)
			result += static_cast<char>(wstr[i] & 0x7F);
		return result;
	}

	// Returns the pixel offset for rulers (0 when rulers are hidden).
	export auto RulerOffset(const DesignState& state) -> int
	{
		return state.showRulers ? RULER_SIZE : 0;
	}

	export auto ControlSubclassProc(
		Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam,
		Win32::UINT_PTR, Win32::DWORD_PTR) -> Win32::LRESULT
	{
		if (msg == Win32::Messages::NcHitTest)
			return Win32::HitTestValues::Transparent;
		return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
	}

	// Destroys and recreates a single control's HWND (e.g. after style change).
	export void RebuildSingleControl(DesignState& state, ControlEntry& entry)
	{
		Win32::DestroyWindow(entry.hwnd);

		auto& ctrl = *entry.control;
		auto* className = FormDesigner::ClassNameFor(ctrl.type);
		if (!className) return;

		auto style = Win32::DWORD{
			Win32::Styles::Child | Win32::Styles::Visible |
			FormDesigner::ImpliedStyleFor(ctrl.type) |
			FormDesigner::AlignmentStyleFor(ctrl.type, ctrl.textAlign) |
			ctrl.style};

		int offset = RulerOffset(state);
		entry.hwnd = Win32::CreateWindowExW(
			ctrl.exStyle, className, ctrl.text.c_str(), style,
			ctrl.rect.x + offset, ctrl.rect.y + offset,
			ctrl.rect.width, ctrl.rect.height,
			state.canvasHwnd,
			reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(ctrl.id)),
			state.hInstance, nullptr);

		if (entry.hwnd)
		{
			Win32::SendMessageW(entry.hwnd, Win32::Messages::SetFont,
				reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);
			Win32::SetWindowSubclass(entry.hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
		}
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
	}

	export auto IsSelected(const DesignState& state, int index) -> bool
	{
		return state.selection.contains(index);
	}

	export auto SingleSelection(const DesignState& state) -> int
	{
		if (state.selection.size() == 1)
			return *state.selection.begin();
		return -1;
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

	export void GetHandleAnchors(const FormDesigner::Rect& r, Win32::POINT out[8])
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
		int sel = SingleSelection(state);
		if (sel < 0 || sel >= static_cast<int>(state.entries.size()))
			return -1;

		Win32::POINT anchors[8];
		GetHandleAnchors(state.entries[sel].control->rect, anchors);

		for (int i = 0; i < 8; ++i)
		{
			if (x >= anchors[i].x && x < anchors[i].x + HANDLE_SIZE &&
				y >= anchors[i].y && y < anchors[i].y + HANDLE_SIZE)
				return i;
		}
		return -1;
	}

	export auto CursorForHandle(int handle) -> Win32::LPCWSTR
	{
		switch (handle)
		{
		case 0: case 7: return Win32::Cursors::SizeNWSE;
		case 2: case 5: return Win32::Cursors::SizeNESW;
		case 1: case 6: return Win32::Cursors::SizeNS;
		case 3: case 4: return Win32::Cursors::SizeWE;
		default:         return Win32::Cursors::Arrow;
		}
	}

	export void ApplyResize(FormDesigner::Rect& r, int handle, int dx, int dy,
		const Win32::POINT& startPos, const Win32::SIZE& startSize)
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

	export void DrawSelection(const DesignState& state, Win32::HDC hdc)
	{
		if (state.selection.empty()) return;

		int offset = RulerOffset(state);
		auto accent = Win32::CreateSolidBrush(state.theme.selectionHighlight);
		auto locked = Win32::CreateSolidBrush(state.theme.lockedHighlight);

		for (int idx : state.selection)
		{
			if (idx < 0 || idx >= static_cast<int>(state.entries.size()))
				continue;

			auto& ctrl = *state.entries[idx].control;
			auto& r = ctrl.rect;
			auto brush = ctrl.locked ? locked : accent;
			int ox = r.x + offset;
			int oy = r.y + offset;

			Win32::RECT sides[] = {
				{ ox - 2, oy - 2,            ox + r.width + 2, oy },
				{ ox - 2, oy + r.height,     ox + r.width + 2, oy + r.height + 2 },
				{ ox - 2, oy,                ox,                oy + r.height },
				{ ox + r.width, oy,          ox + r.width + 2,  oy + r.height },
			};
			for (auto& s : sides)
				Win32::FillRect(hdc, &s, brush);
		}

		// Resize handles only when exactly one unlocked control is selected.
		int sel = SingleSelection(state);
		if (sel >= 0 && sel < static_cast<int>(state.entries.size()) &&
			!state.entries[sel].control->locked)
		{
			auto& r = state.entries[sel].control->rect;
			Win32::POINT anchors[8];
			GetHandleAnchors(r, anchors);

			auto handleBrush = Win32::CreateSolidBrush(state.theme.handleFill);
			for (auto& a : anchors)
			{
				Win32::RECT outer = { a.x + offset, a.y + offset,
									a.x + HANDLE_SIZE + offset, a.y + HANDLE_SIZE + offset };
				Win32::RECT inner = { a.x + 1 + offset, a.y + 1 + offset,
									a.x + HANDLE_SIZE - 1 + offset, a.y + HANDLE_SIZE - 1 + offset };
				Win32::FillRect(hdc, &outer, accent);
				Win32::FillRect(hdc, &inner, handleBrush);
			}
			Win32::DeleteObject(handleBrush);
		}

		Win32::DeleteObject(locked);
		Win32::DeleteObject(accent);
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
		Win32::SetWindowTextW(state.surfaceHwnd, title.c_str());
	}

	export void UpdateStatusBar(DesignState& state)
	{
		if (!state.statusbarHwnd) return;

		// Pane 0: selection info.
		int sel = SingleSelection(state);
		std::wstring info;
		if (sel >= 0 && sel < static_cast<int>(state.entries.size()))
		{
			auto& ctrl = *state.entries[sel].control;
			auto typeName = ControlTypeDisplayName(ctrl.type);
			auto text = ctrl.text.size() > 20
				? ctrl.text.substr(0, 17) + L"..."
				: ctrl.text;
			info = std::format(L"{} '{}' ({},{} {}x{})",
				typeName, text,
				ctrl.rect.x, ctrl.rect.y, ctrl.rect.width, ctrl.rect.height);
		}
		else if (state.selection.size() > 1)
		{
			info = std::format(L"{} controls selected", state.selection.size());
		}
		else
		{
			auto& title = state.form.title;
			info = std::format(L"Form: {}", title);
		}
		Win32::SendMessageW(state.statusbarHwnd, Win32::StatusBar::SetTextW, 0,
			reinterpret_cast<Win32::LPARAM>(info.c_str()));

		// Pane 2: file state.
		auto fileState = state.dirty ? L"Modified" : L"Ready";
		Win32::SendMessageW(state.statusbarHwnd, Win32::StatusBar::SetTextW, 2,
			reinterpret_cast<Win32::LPARAM>(fileState));
	}

	export void MarkDirty(DesignState& state)
	{
		if (!state.dirty)
		{
			state.dirty = true;
			UpdateTitle(state);
		}
		UpdateStatusBar(state);
	}

	// Paints ruler contents into a memory DC (called by DrawRulers).
	void PaintRulersToBuffer(const DesignState& state, Win32::HDC memDC, int canvasW, int canvasH)
	{
		auto font = Win32::CreateFontW(
			11, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, L"Segoe UI");
		auto oldFont = Win32::SelectObject(memDC, font);
		auto oldTextColor = Win32::SetTextColor(memDC, state.theme.rulerText);
		auto oldBkMode = Win32::SetBkMode(memDC, Win32::Bk_Transparent);

		auto rulerBrush = Win32::CreateSolidBrush(state.theme.rulerBackground);
		Win32::RECT topRuler = { RULER_SIZE, 0, canvasW, RULER_SIZE };
		Win32::RECT leftRuler = { 0, RULER_SIZE, RULER_SIZE, canvasH };
		Win32::RECT corner = { 0, 0, RULER_SIZE, RULER_SIZE };
		Win32::FillRect(memDC, &topRuler, rulerBrush);
		Win32::FillRect(memDC, &leftRuler, rulerBrush);
		Win32::FillRect(memDC, &corner, rulerBrush);
		Win32::DeleteObject(rulerBrush);

		auto borderPen = Win32::CreatePen(0, 1, state.theme.rulerBorder);
		auto oldPen = Win32::SelectObject(memDC, borderPen);
		Win32::MoveToEx(memDC, RULER_SIZE, 0, nullptr);
		Win32::LineTo(memDC, RULER_SIZE, canvasH);
		Win32::MoveToEx(memDC, 0, RULER_SIZE, nullptr);
		Win32::LineTo(memDC, canvasW, RULER_SIZE);
		Win32::SelectObject(memDC, oldPen);
		Win32::DeleteObject(borderPen);

		auto tickPen = Win32::CreatePen(0, 1, state.theme.rulerTick);
		oldPen = Win32::SelectObject(memDC, tickPen);

		for (int px = 0; px < canvasW - RULER_SIZE; px += 10)
		{
			int screenX = px + RULER_SIZE;
			if (px % 50 == 0)
			{
				Win32::MoveToEx(memDC, screenX, RULER_SIZE - 8, nullptr);
				Win32::LineTo(memDC, screenX, RULER_SIZE);
				auto label = std::to_wstring(px);
				Win32::TextOutW(memDC, screenX + 2, 1, label.c_str(), static_cast<int>(label.size()));
			}
			else
			{
				Win32::MoveToEx(memDC, screenX, RULER_SIZE - 4, nullptr);
				Win32::LineTo(memDC, screenX, RULER_SIZE);
			}
		}

		for (int py = 0; py < canvasH - RULER_SIZE; py += 10)
		{
			int screenY = py + RULER_SIZE;
			if (py % 50 == 0)
			{
				Win32::MoveToEx(memDC, RULER_SIZE - 8, screenY, nullptr);
				Win32::LineTo(memDC, RULER_SIZE, screenY);
				auto label = std::to_wstring(py);
				Win32::TextOutW(memDC, 1, screenY + 2, label.c_str(), static_cast<int>(label.size()));
			}
			else
			{
				Win32::MoveToEx(memDC, RULER_SIZE - 4, screenY, nullptr);
				Win32::LineTo(memDC, RULER_SIZE, screenY);
			}
		}

		Win32::SelectObject(memDC, oldPen);
		Win32::DeleteObject(tickPen);
		Win32::SetBkMode(memDC, oldBkMode);
		Win32::SetTextColor(memDC, oldTextColor);
		Win32::SelectObject(memDC, oldFont);
		Win32::DeleteObject(font);
	}

	// Paints cursor indicator lines into a memory DC.
	void PaintCursorIndicatorToBuffer(const DesignState& state, Win32::HDC memDC, int formX, int formY)
	{
		int offset = RULER_SIZE;
		auto pen = Win32::CreatePen(0, 1, state.theme.formBoundary);
		auto oldPen = Win32::SelectObject(memDC, pen);

		int sx = formX + offset;
		Win32::MoveToEx(memDC, sx, 0, nullptr);
		Win32::LineTo(memDC, sx, RULER_SIZE);

		int sy = formY + offset;
		Win32::MoveToEx(memDC, 0, sy, nullptr);
		Win32::LineTo(memDC, RULER_SIZE, sy);

		Win32::SelectObject(memDC, oldPen);
		Win32::DeleteObject(pen);
	}

	export void DrawRulers(const DesignState& state, Win32::HDC hdc)
	{
		if (!state.showRulers) return;

		Win32::RECT rc;
		Win32::GetClientRect(state.canvasHwnd, &rc);
		int w = rc.right;
		int h = rc.bottom;

		// Double-buffer: draw rulers + indicator to off-screen bitmap, then blit.
		auto memDC = Win32::CreateCompatibleDC(hdc);
		auto memBmp = Win32::CreateCompatibleBitmap(hdc, w, h);
		auto oldBmp = Win32::SelectObject(memDC, memBmp);

		PaintRulersToBuffer(state, memDC, w, h);

		if (state.lastCursorPos.x >= 0)
			PaintCursorIndicatorToBuffer(state, memDC, state.lastCursorPos.x, state.lastCursorPos.y);

		// Blit top ruler strip.
		Win32::BitBlt(hdc, 0, 0, w, RULER_SIZE, memDC, 0, 0, Win32::SrcCopy);
		// Blit left ruler strip.
		Win32::BitBlt(hdc, 0, RULER_SIZE, RULER_SIZE, h - RULER_SIZE, memDC, 0, RULER_SIZE, Win32::SrcCopy);

		Win32::SelectObject(memDC, oldBmp);
		Win32::DeleteObject(memBmp);
		Win32::DeleteDC(memDC);
	}

	export void DrawRulerCursorIndicator(const DesignState& state, Win32::HDC hdc, int formX, int formY)
	{
		// Now handled inside DrawRulers() via the double buffer - kept for API compatibility.
		(void)state; (void)hdc; (void)formX; (void)formY;
	}

	export void DrawUserGuides(const DesignState& state, Win32::HDC hdc)
	{
		if (state.userGuides.empty() && state.dragMode != DragMode::CreateGuide) return;

		int offset = RulerOffset(state);
		Win32::RECT rc;
		Win32::GetClientRect(state.canvasHwnd, &rc);

		auto pen = Win32::CreatePen(Win32::PenStyles::Dash, 0, state.theme.userGuide);
		auto oldPen = Win32::SelectObject(hdc, pen);
		auto oldMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);

		for (auto& guide : state.userGuides)
		{
			if (guide.horizontal)
			{
				Win32::MoveToEx(hdc, 0, guide.position + offset, nullptr);
				Win32::LineTo(hdc, rc.right, guide.position + offset);
			}
			else
			{
				Win32::MoveToEx(hdc, guide.position + offset, 0, nullptr);
				Win32::LineTo(hdc, guide.position + offset, rc.bottom);
			}
		}

		// Draw the guide being dragged (preview line).
		if (state.dragMode == DragMode::CreateGuide && state.draggingGuidePos >= 0)
		{
			if (state.draggingGuideHorizontal)
			{
				Win32::MoveToEx(hdc, 0, state.draggingGuidePos + offset, nullptr);
				Win32::LineTo(hdc, rc.right, state.draggingGuidePos + offset);
			}
			else
			{
				Win32::MoveToEx(hdc, state.draggingGuidePos + offset, 0, nullptr);
				Win32::LineTo(hdc, state.draggingGuidePos + offset, rc.bottom);
			}
		}

		Win32::SetBkMode(hdc, oldMode);
		Win32::SelectObject(hdc, oldPen);
		Win32::DeleteObject(pen);
	}

	// Hit test for existing user guides (returns index or -1).
	export auto HitTestUserGuide(const DesignState& state, int formX, int formY) -> int
	{
		for (int i = 0; i < static_cast<int>(state.userGuides.size()); ++i)
		{
			auto& g = state.userGuides[i];
			int coord = g.horizontal ? formY : formX;
			if (std::abs(coord - g.position) <= 3)
				return i;
		}
		return -1;
	}

	export auto ControlTypeDisplayName(FormDesigner::ControlType type) -> const wchar_t*
	{
		switch (type)
		{
		case FormDesigner::ControlType::Button:      return L"Button";
		case FormDesigner::ControlType::CheckBox:    return L"CheckBox";
		case FormDesigner::ControlType::RadioButton: return L"RadioButton";
		case FormDesigner::ControlType::Label:       return L"Label";
		case FormDesigner::ControlType::TextBox:     return L"TextBox";
		case FormDesigner::ControlType::GroupBox:     return L"GroupBox";
		case FormDesigner::ControlType::ListBox:     return L"ListBox";
		case FormDesigner::ControlType::ComboBox:    return L"ComboBox";
		case FormDesigner::ControlType::ProgressBar:     return L"ProgressBar";
		case FormDesigner::ControlType::TrackBar:        return L"TrackBar";
		case FormDesigner::ControlType::DateTimePicker:  return L"DateTimePicker";
		case FormDesigner::ControlType::TabControl:      return L"TabControl";
		case FormDesigner::ControlType::ListView:        return L"ListView";
		case FormDesigner::ControlType::TreeView:        return L"TreeView";
		case FormDesigner::ControlType::UpDown:          return L"UpDown";
		case FormDesigner::ControlType::RichEdit:        return L"RichEdit";
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

	export void PushUndo(DesignState& state)
	{
		state.undoStack.push_back(state.form);
		state.redoStack.clear();
	}

	export int SnapValue(int value, int gridSize)
	{
		return ((value + gridSize / 2) / gridSize) * gridSize;
	}

	export void SnapRectToGrid(FormDesigner::Rect& rect, int gridSize)
	{
		rect.x = SnapValue(rect.x, gridSize);
		rect.y = SnapValue(rect.y, gridSize);
	}

	// Compares edges of the moving control against all other controls.
	// Snaps the rect in-place when within threshold and populates state.guides.
	export void FindAlignGuides(DesignState& state, FormDesigner::Rect& rect)
	{
		state.guides.clear();

		// 5 reference values for the moving control: left, center-x, right, top, center-y, bottom
		auto edges = [](const FormDesigner::Rect& r)
		{
			return std::array<int, 6>{
				r.x,
				r.x + r.width / 2,
				r.x + r.width,
				r.y,
				r.y + r.height / 2,
				r.y + r.height,
			};
		};

		bool snappedX = false;
		bool snappedY = false;

		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
		{
			if (state.selection.contains(i)) continue;
			auto& other = state.entries[i].control->rect;
			auto otherEdges = edges(other);

			auto movingEdges = edges(rect);

			// Check vertical alignment (shared X positions)
			if (!snappedX)
			{
				for (int m = 0; m < 3; ++m)
				{
					for (int o = 0; o < 3; ++o)
					{
						int diff = movingEdges[m] - otherEdges[o];
						if (std::abs(diff) <= SNAP_THRESHOLD)
						{
							// Snap: adjust rect.x so moving edge m aligns with other edge o
							rect.x -= diff;
							state.guides.push_back({ false, otherEdges[o] });
							snappedX = true;
							break;
						}
					}
					if (snappedX) break;
				}
			}

			// Check horizontal alignment (shared Y positions)
			if (!snappedY)
			{
				for (int m = 3; m < 6; ++m)
				{
					for (int o = 3; o < 6; ++o)
					{
						int diff = movingEdges[m] - otherEdges[o];
						if (std::abs(diff) <= SNAP_THRESHOLD)
						{
							rect.y -= diff;
							state.guides.push_back({ true, otherEdges[o] });
							snappedY = true;
							break;
						}
					}
					if (snappedY) break;
				}
			}

			if (snappedX && snappedY) break;
		}

		// Snap to user guides.
		if (!snappedX || !snappedY)
		{
			auto movingEdges = edges(rect);
			for (auto& ug : state.userGuides)
			{
				if (!ug.horizontal && !snappedX)
				{
					for (int m = 0; m < 3; ++m)
					{
						int diff = movingEdges[m] - ug.position;
						if (std::abs(diff) <= SNAP_THRESHOLD)
						{
							rect.x -= diff;
							state.guides.push_back({ false, ug.position });
							snappedX = true;
							break;
						}
					}
				}
				if (ug.horizontal && !snappedY)
				{
					for (int m = 3; m < 6; ++m)
					{
						int diff = movingEdges[m] - ug.position;
						if (std::abs(diff) <= SNAP_THRESHOLD)
						{
							rect.y -= diff;
							state.guides.push_back({ true, ug.position });
							snappedY = true;
							break;
						}
					}
				}
				if (snappedX && snappedY) break;
			}
		}
	}

	export void DrawAlignGuides(const DesignState& state, Win32::HDC hdc)
	{
		if (state.guides.empty()) return;

		int offset = RulerOffset(state);
		Win32::RECT rc;
		Win32::GetClientRect(state.canvasHwnd, &rc);

		auto pen = Win32::CreatePen(Win32::PenStyles::Dot, 0, state.theme.alignGuide);
		auto oldPen = Win32::SelectObject(hdc, pen);
		auto oldMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);

		for (auto& guide : state.guides)
		{
			if (guide.horizontal)
			{
				Win32::MoveToEx(hdc, rc.left, guide.position + offset, nullptr);
				Win32::LineTo(hdc, rc.right, guide.position + offset);
			}
			else
			{
				Win32::MoveToEx(hdc, guide.position + offset, rc.top, nullptr);
				Win32::LineTo(hdc, guide.position + offset, rc.bottom);
			}
		}

		Win32::SetBkMode(hdc, oldMode);
		Win32::SelectObject(hdc, oldPen);
		Win32::DeleteObject(pen);
	}

	export auto GetSettingsPath() -> std::filesystem::path
	{
		wchar_t exePath[Win32::MaxPath] = {};
		Win32::GetModuleFileNameW(nullptr, exePath, Win32::MaxPath);
		auto p = std::filesystem::path(exePath).parent_path() / L"designer.ini";
		return p;
	}

	export void SaveThemePreference(bool isDark)
	{
		auto path = GetSettingsPath();
		auto out = std::ofstream(path);
		if (out)
			out << (isDark ? "dark" : "light") << std::endl;
	}

	export auto LoadThemePreference() -> bool
	{
		auto path = GetSettingsPath();
		auto in = std::ifstream(path);
		if (in)
		{
			auto line = std::string{};
			std::getline(in, line);
			return line == "dark";
		}
		return false;
	}

}
