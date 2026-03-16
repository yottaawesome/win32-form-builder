export module designer:helpers;
import std;
import formbuilder;
import :state;
import :hit_testing;

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

	// Checks whether a wide string is a valid C++ identifier (or empty).
	export auto IsValidIdentifier(const wchar_t* str) -> bool
	{
		if (!str || str[0] == L'\0') return true; // empty is valid (optional field)
		if (!(str[0] == L'_' || (str[0] >= L'A' && str[0] <= L'Z') ||
			(str[0] >= L'a' && str[0] <= L'z')))
			return false;
		for (int i = 1; str[i] != L'\0'; ++i)
		{
			auto ch = str[i];
			if (!(ch == L'_' || (ch >= L'A' && ch <= L'Z') ||
				(ch >= L'a' && ch <= L'z') || (ch >= L'0' && ch <= L'9')))
				return false;
		}
		return true;
	}

	// Checks whether a control ID is used by another control.
	export auto IsDuplicateId(const DesignState& state, int id, int excludeIndex) -> bool
	{
		if (id == 0) return false; // 0 is a common default, allow duplicates
		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
		{
			if (i == excludeIndex) continue;
			if (state.entries[i].control->id == id)
				return true;
		}
		return false;
	}

	// Returns the pixel offset for rulers (0 when rulers are hidden).
	export auto RulerOffset(const DesignState& state) -> int
	{
		return state.showRulers ? state.dpiInfo.RulerSize() : 0;
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
			// Apply resolved font.
			auto resolved = FormDesigner::ResolveFont(ctrl.font, state.form.font);
			if (resolved.family != FormDesigner::DefaultFontFamily || resolved.size != FormDesigner::DefaultFontSize
				|| resolved.bold || resolved.italic)
			{
				auto hFont = Win32::CreateFontFromInfo(
					resolved.family.c_str(), resolved.size, resolved.bold, resolved.italic, entry.hwnd);
				state.controlFonts.push_back(hFont);
				Win32::SendMessageW(entry.hwnd, Win32::Messages::SetFont,
					reinterpret_cast<Win32::WPARAM>(hFont), true);
			}
			else
			{
				Win32::SendMessageW(entry.hwnd, Win32::Messages::SetFont,
					reinterpret_cast<Win32::WPARAM>(Win32::GetStockObject(Win32::DefaultGuiFont)), true);
			}
			Win32::SetWindowSubclass(entry.hwnd, ControlSubclassProc, SUBCLASS_ID, 0);
		}
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
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
		case FormDesigner::ControlType::MonthCalendar:   return L"Month Calendar";
		case FormDesigner::ControlType::Link:            return L"Link";
		case FormDesigner::ControlType::IPAddress:       return L"IP Address";
		case FormDesigner::ControlType::HotKey:          return L"Hot Key";
		case FormDesigner::ControlType::Picture:         return L"Picture";
		case FormDesigner::ControlType::Separator:       return L"Separator";
		case FormDesigner::ControlType::Animation:       return L"Animation";
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

	// Assigns a new shared group ID to all selected controls.
	export void GroupSelected(DesignState& state)
	{
		if (state.selection.size() < 2) return;
		PushUndo(state);
		int gid = state.nextGroupId++;
		for (int idx : state.selection)
		{
			if (idx >= 0 && idx < static_cast<int>(state.entries.size()))
				state.entries[idx].control->groupId = gid;
		}
		MarkDirty(state);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
	}

	// Removes group membership from all selected controls.
	export void UngroupSelected(DesignState& state)
	{
		if (state.selection.empty()) return;
		PushUndo(state);
		for (int idx : state.selection)
		{
			if (idx >= 0 && idx < static_cast<int>(state.entries.size()))
				state.entries[idx].control->groupId = 0;
		}
		MarkDirty(state);
		Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
	}

	// Recalculates nextGroupId from existing controls (call after loading a file).
	export void SyncNextGroupId(DesignState& state)
	{
		int maxId = 0;
		for (auto& c : state.form.controls)
			if (c.groupId > maxId) maxId = c.groupId;
		state.nextGroupId = maxId + 1;
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
						if (std::abs(diff) <= state.dpiInfo.SnapThreshold())
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
						if (std::abs(diff) <= state.dpiInfo.SnapThreshold())
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
						if (std::abs(diff) <= state.dpiInfo.SnapThreshold())
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
						if (std::abs(diff) <= state.dpiInfo.SnapThreshold())
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
	// Sorts controls by position (top-to-bottom, left-to-right) and assigns
	// sequential tab indices starting from 1.
	export void AutoAssignTabOrder(DesignState& state)
	{
		int count = static_cast<int>(state.form.controls.size());
		if (count == 0) return;

		// Build index list sorted by (y, x).
		auto indices = std::vector<int>(count);
		std::iota(indices.begin(), indices.end(), 0);
		std::sort(indices.begin(), indices.end(), [&](int a, int b)
		{
			auto& ra = state.form.controls[a].rect;
			auto& rb = state.form.controls[b].rect;
			if (ra.y != rb.y) return ra.y < rb.y;
			return ra.x < rb.x;
		});

		for (int i = 0; i < count; ++i)
			state.form.controls[indices[i]].tabIndex = i + 1;
	}
}
