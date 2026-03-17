export module designer:properties;
import std;
import formbuilder;
import :state;
import :hit_testing;
import :helpers;

namespace Designer
{

	void UpdateScrollRange(DesignState& state);
	void ScrollPropertyPanel(DesignState& state, int newPos);
	void ResetPropertyScroll(DesignState& state);

	// Formats a FontInfo into a display string like "Segoe UI, 12pt, Bold Italic" or "(inherited)".
	auto FontDisplayString(const FormDesigner::FontInfo& font) -> std::wstring
	{
		if (!font.isSet())
			return L"(inherited)";

		auto result = std::wstring{};
		if (!font.family.empty())
			result = font.family;
		else
			result = L"(inherited)";

		if (font.size != 0)
			result += std::format(L", {}pt", font.size);

		if (font.bold)
			result += L", Bold";
		if (font.italic)
			result += L", Italic";

		return result;
	}

	auto FormFontDisplayString(const FormDesigner::FontInfo& font) -> std::wstring
	{
		if (!font.isSet())
			return L"(default)";
		return FontDisplayString(font);
	}

	void SetPropertyGroupVisibility(Win32::HWND panel, const Win32::UINT ids[], int count, int show)
	{
		for (int i = 0; i < count; ++i)
		{
			auto edit = Win32::GetDlgItem(panel, ids[i]);
			if (edit) Win32::ShowWindow(edit, show);
			auto label = Win32::GetDlgItem(panel, ids[i] + IDL_OFFSET);
			if (label) Win32::ShowWindow(label, show);
		}
	}

	void UpdatePropertyVisibility(DesignState& state, Win32::HWND panel, bool hasSel)
	{
		constexpr Win32::UINT ctrlIds[] = {
			IDC_PROP_TYPE, IDC_PROP_TEXT, IDC_PROP_ID,
			IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H,
			IDC_PROP_ONCLICK, IDC_PROP_ONCHANGE, IDC_PROP_ONDBLCLICK, IDC_PROP_ONSELCHANGE,
			IDC_PROP_ONFOCUS, IDC_PROP_ONBLUR, IDC_PROP_ONCHECK, IDC_PROP_TABINDEX,
			IDC_PROP_TEXTALIGN, IDC_PROP_LOCKED, IDC_PROP_VISIBLE, IDC_PROP_ENABLED, IDC_PROP_ANCHOR, IDC_PROP_TOOLTIP,
			IDC_PROP_ACCESSNAME, IDC_PROP_ACCESSDESC
		};
		constexpr Win32::UINT formIds[] = {
			IDC_PROP_FORM_TITLE, IDC_PROP_FORM_WIDTH,
			IDC_PROP_FORM_HEIGHT, IDC_PROP_FORM_BGCOLOR,
			IDC_PROP_FORM_CAPTION, IDC_PROP_FORM_SYSMENU,
			IDC_PROP_FORM_RESIZABLE, IDC_PROP_FORM_MINIMIZE, IDC_PROP_FORM_MAXIMIZE,
			IDC_PROP_FORM_BINDSTRUCT
		};

		int ctrlShow = hasSel ? Win32::Sw_Show : Win32::Sw_Hide;
		int formShow = hasSel ? Win32::Sw_Hide : Win32::Sw_Show;

		SetPropertyGroupVisibility(panel, ctrlIds, 23, ctrlShow);
		SetPropertyGroupVisibility(panel, formIds, 10, formShow);

		auto bgBtn = Win32::GetDlgItem(panel, IDC_PROP_FORM_BGCOLOR_BTN);
		if (bgBtn) Win32::ShowWindow(bgBtn, formShow);

		// Control font controls.
		Win32::UINT ctrlFontIds[] = { IDC_PROP_FONT_LABEL, IDC_PROP_FONT_BTN, IDC_PROP_FONT_CLEAR };
		for (auto id : ctrlFontIds)
		{
			auto h = Win32::GetDlgItem(panel, id);
			if (h) Win32::ShowWindow(h, ctrlShow);
		}

		// Form font controls.
		Win32::UINT formFontIds[] = { IDC_PROP_FORM_FONT_LABEL, IDC_PROP_FORM_FONT_BTN, IDC_PROP_FORM_FONT_CLEAR };
		for (auto id : formFontIds)
		{
			auto h = Win32::GetDlgItem(panel, id);
			if (h) Win32::ShowWindow(h, formShow);
		}

		// Font label headers.
		auto fontLblHdr = Win32::GetDlgItem(panel, IDC_PROP_FONT_LABEL + IDL_OFFSET);
		if (fontLblHdr) Win32::ShowWindow(fontLblHdr, ctrlShow);
		auto formFontLblHdr = Win32::GetDlgItem(panel, IDC_PROP_FORM_FONT_LABEL + IDL_OFFSET);
		if (formFontLblHdr) Win32::ShowWindow(formFontLblHdr, formShow);

		// Items controls — visible only for ComboBox/ListBox.
		int sel = SingleSelection(state);
		bool isListType = false;
		if (hasSel && sel >= 0 && sel < static_cast<int>(state.entries.size()))
		{
			auto t = state.entries[sel].control->type;
			isListType = (t == FormDesigner::ControlType::ComboBox ||
						  t == FormDesigner::ControlType::ListBox);
		}
		int itemsShow = isListType ? Win32::Sw_Show : Win32::Sw_Hide;
		Win32::UINT itemsIds[] = { IDC_PROP_ITEMS_LABEL, IDC_PROP_ITEMS_BTN, IDC_PROP_SELINDEX };
		for (auto id : itemsIds)
		{
			auto h = Win32::GetDlgItem(panel, id);
			if (h) Win32::ShowWindow(h, itemsShow);
			auto lbl = Win32::GetDlgItem(panel, id + IDL_OFFSET);
			if (lbl) Win32::ShowWindow(lbl, itemsShow);
		}

		// Validation controls — show relevant fields by control type.
		auto ctype = FormDesigner::ControlType::Window;
		if (hasSel && sel >= 0 && sel < static_cast<int>(state.entries.size()))
			ctype = state.entries[sel].control->type;

		bool showRequired = hasSel && FormDesigner::SupportsRequiredValidation(ctype);
		bool showTextVal  = hasSel && FormDesigner::SupportsTextValidation(ctype);
		bool showRangeVal = hasSel && FormDesigner::SupportsRangeValidation(ctype);

		auto showId = [&](Win32::UINT id, bool show) {
			auto h = Win32::GetDlgItem(panel, id);
			if (h) Win32::ShowWindow(h, show ? Win32::Sw_Show : Win32::Sw_Hide);
			auto lbl = Win32::GetDlgItem(panel, id + IDL_OFFSET);
			if (lbl) Win32::ShowWindow(lbl, show ? Win32::Sw_Show : Win32::Sw_Hide);
		};

		showId(IDC_PROP_VAL_REQUIRED, showRequired);
		showId(IDC_PROP_VAL_MINLEN, showTextVal);
		showId(IDC_PROP_VAL_MAXLEN, showTextVal);
		showId(IDC_PROP_VAL_PATTERN, showTextVal);
		showId(IDC_PROP_VAL_MIN, showRangeVal);
		showId(IDC_PROP_VAL_MAX, showRangeVal);

		// Value control — visible for ProgressBar, TrackBar, UpDown.
		bool showValue = hasSel && FormDesigner::SupportsValue(ctype);
		showId(IDC_PROP_VALUE, showValue);

		// Image path controls — visible only for Picture type.
		bool showImage = hasSel && ctype == FormDesigner::ControlType::Picture;
		showId(IDC_PROP_IMAGEPATH, showImage);
		auto imgBtn = Win32::GetDlgItem(panel, IDC_PROP_IMAGEPATH_BTN);
		if (imgBtn) Win32::ShowWindow(imgBtn, showImage ? Win32::Sw_Show : Win32::Sw_Hide);

		// Data binding field — visible for any selected control.
		showId(IDC_PROP_BINDFIELD, hasSel);

		// Accessibility fields — visible for any selected control.
		showId(IDC_PROP_ACCESSNAME, hasSel);
		showId(IDC_PROP_ACCESSDESC, hasSel);
		// TabStop/GroupStart checkboxes — no labels via IDL_OFFSET, just show/hide directly.
		auto tabStopChk = Win32::GetDlgItem(panel, IDC_PROP_TABSTOP);
		if (tabStopChk) Win32::ShowWindow(tabStopChk, ctrlShow);
		auto groupStartChk = Win32::GetDlgItem(panel, IDC_PROP_GROUPSTART);
		if (groupStartChk) Win32::ShowWindow(groupStartChk, ctrlShow);
	}

	void UpdateControlProperties(DesignState& state, Win32::HWND panel, int sel)
	{
		auto& ctrl = *state.entries[sel].control;

		Win32::SetDlgItemTextW(panel, IDC_PROP_TYPE, ControlTypeDisplayName(ctrl.type));
		Win32::SetDlgItemTextW(panel, IDC_PROP_TEXT, ctrl.text.c_str());
		Win32::SetDlgItemInt(panel, IDC_PROP_ID, static_cast<Win32::UINT>(ctrl.id), false);
		Win32::SetDlgItemInt(panel, IDC_PROP_X, ctrl.rect.x, true);
		Win32::SetDlgItemInt(panel, IDC_PROP_Y, ctrl.rect.y, true);
		Win32::SetDlgItemInt(panel, IDC_PROP_W, ctrl.rect.width, true);
		Win32::SetDlgItemInt(panel, IDC_PROP_H, ctrl.rect.height, true);

		// Event handler fields.
		struct EventField { Win32::UINT id; const std::string& value; };
		EventField events[] = {
			{ IDC_PROP_ONCLICK,    ctrl.onClick },
			{ IDC_PROP_ONCHANGE,   ctrl.onChange },
			{ IDC_PROP_ONDBLCLICK, ctrl.onDoubleClick },
			{ IDC_PROP_ONSELCHANGE, ctrl.onSelectionChange },
			{ IDC_PROP_ONFOCUS,    ctrl.onFocus },
			{ IDC_PROP_ONBLUR,     ctrl.onBlur },
			{ IDC_PROP_ONCHECK,    ctrl.onCheck },
		};
		for (auto& [id, value] : events)
		{
			auto wide = std::wstring(value.begin(), value.end());
			Win32::SetDlgItemTextW(panel, id, wide.c_str());
		}

		Win32::SetDlgItemInt(panel, IDC_PROP_TABINDEX, static_cast<Win32::UINT>(ctrl.tabIndex), true);

		auto alignCombo = Win32::GetDlgItem(panel, IDC_PROP_TEXTALIGN);
		if (alignCombo)
			Win32::SendMessageW(alignCombo, Win32::ComboBox::SetCurSel,
				static_cast<Win32::WPARAM>(ctrl.textAlign), 0);

		Win32::SendMessageW(Win32::GetDlgItem(panel, IDC_PROP_LOCKED),
			Win32::Button::SetCheck,
			ctrl.locked ? Win32::Button::Checked : Win32::Button::Unchecked, 0);

		Win32::SendMessageW(Win32::GetDlgItem(panel, IDC_PROP_VISIBLE),
			Win32::Button::SetCheck,
			ctrl.visible ? Win32::Button::Checked : Win32::Button::Unchecked, 0);

		Win32::SendMessageW(Win32::GetDlgItem(panel, IDC_PROP_ENABLED),
			Win32::Button::SetCheck,
			ctrl.enabled ? Win32::Button::Checked : Win32::Button::Unchecked, 0);

		// Anchor dropdown.
		{
			using namespace FormDesigner::Anchor;
			constexpr int anchorValues[] = {
				Top | Left, Top | Right, Bottom | Left, Bottom | Right,
				Top | Left | Right, Bottom | Left | Right,
				Top | Bottom | Left, Top | Bottom | Right,
				Top | Bottom | Left | Right,
			};
			int anchorIdx = 0;
			for (int i = 0; i < 9; ++i)
				if (anchorValues[i] == ctrl.anchor) { anchorIdx = i; break; }
			Win32::SendMessageW(Win32::GetDlgItem(panel, IDC_PROP_ANCHOR),
				Win32::ComboBox::SetCurSel, anchorIdx, 0);
		}

		Win32::SetDlgItemTextW(panel, IDC_PROP_FONT_LABEL,
			FontDisplayString(ctrl.font).c_str());

		Win32::SetDlgItemTextW(panel, IDC_PROP_TOOLTIP, ctrl.tooltip.c_str());

		// Items summary label and selectedIndex.
		auto itemsLabel = std::format(L"{} item{}", ctrl.items.size(),
			ctrl.items.size() == 1 ? L"" : L"s");
		Win32::SetDlgItemTextW(panel, IDC_PROP_ITEMS_LABEL, itemsLabel.c_str());
		Win32::SetDlgItemInt(panel, IDC_PROP_SELINDEX,
			static_cast<Win32::UINT>(ctrl.selectedIndex < 0 ? -1 : ctrl.selectedIndex), true);

		// Validation fields.
		Win32::SendMessageW(Win32::GetDlgItem(panel, IDC_PROP_VAL_REQUIRED),
			Win32::Button::SetCheck,
			ctrl.validation.required ? Win32::Button::Checked : Win32::Button::Unchecked, 0);
		Win32::SetDlgItemInt(panel, IDC_PROP_VAL_MINLEN,
			static_cast<Win32::UINT>(ctrl.validation.minLength), false);
		Win32::SetDlgItemInt(panel, IDC_PROP_VAL_MAXLEN,
			static_cast<Win32::UINT>(ctrl.validation.maxLength), false);
		{
			auto patW = std::wstring(ctrl.validation.pattern.begin(), ctrl.validation.pattern.end());
			Win32::SetDlgItemTextW(panel, IDC_PROP_VAL_PATTERN, patW.c_str());
		}
		Win32::SetDlgItemInt(panel, IDC_PROP_VAL_MIN, ctrl.validation.min, true);
		Win32::SetDlgItemInt(panel, IDC_PROP_VAL_MAX, ctrl.validation.max, true);

		// Value field.
		Win32::SetDlgItemInt(panel, IDC_PROP_VALUE,
			static_cast<Win32::UINT>(ctrl.value), true);

		// Image path field (Picture only).
		Win32::SetDlgItemTextW(panel, IDC_PROP_IMAGEPATH, ctrl.imagePath.c_str());

		// Data binding field.
		{
			auto bindW = std::wstring(ctrl.bindField.begin(), ctrl.bindField.end());
			Win32::SetDlgItemTextW(panel, IDC_PROP_BINDFIELD, bindW.c_str());
		}

		// Accessibility fields.
		Win32::SetDlgItemTextW(panel, IDC_PROP_ACCESSNAME, ctrl.accessibleName.c_str());
		Win32::SetDlgItemTextW(panel, IDC_PROP_ACCESSDESC, ctrl.accessibleDescription.c_str());

		Win32::SendMessageW(Win32::GetDlgItem(panel, IDC_PROP_TABSTOP),
			Win32::Button::SetCheck,
			ctrl.tabStop ? Win32::Button::Checked : Win32::Button::Unchecked, 0);

		Win32::SendMessageW(Win32::GetDlgItem(panel, IDC_PROP_GROUPSTART),
			Win32::Button::SetCheck,
			ctrl.groupStart ? Win32::Button::Checked : Win32::Button::Unchecked, 0);

		Win32::UINT editableIds[] = { IDC_PROP_TEXT, IDC_PROP_ID,
			IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H,
			IDC_PROP_ONCLICK, IDC_PROP_ONCHANGE, IDC_PROP_ONDBLCLICK, IDC_PROP_ONSELCHANGE,
			IDC_PROP_ONFOCUS, IDC_PROP_ONBLUR, IDC_PROP_ONCHECK, IDC_PROP_TABINDEX,
			IDC_PROP_TOOLTIP, IDC_PROP_SELINDEX,
			IDC_PROP_VAL_MINLEN, IDC_PROP_VAL_MAXLEN, IDC_PROP_VAL_PATTERN,
			IDC_PROP_VAL_MIN, IDC_PROP_VAL_MAX, IDC_PROP_VALUE,
			IDC_PROP_IMAGEPATH, IDC_PROP_BINDFIELD,
			IDC_PROP_ACCESSNAME, IDC_PROP_ACCESSDESC };
		for (auto id : editableIds)
			Win32::EnableWindow(Win32::GetDlgItem(panel, id), true);
	}

	void UpdateFormProperties(DesignState& state, Win32::HWND panel)
	{
		Win32::SetDlgItemTextW(panel, IDC_PROP_FORM_TITLE, state.form.title.c_str());
		Win32::SetDlgItemInt(panel, IDC_PROP_FORM_WIDTH, state.form.width, false);
		Win32::SetDlgItemInt(panel, IDC_PROP_FORM_HEIGHT, state.form.height, false);
		Win32::SetDlgItemTextW(panel, IDC_PROP_FORM_BGCOLOR,
			ColorRefToHex(state.form.backgroundColor).c_str());

		struct StyleBit { Win32::UINT id; Win32::DWORD flag; };
		StyleBit bits[] = {
			{ IDC_PROP_FORM_CAPTION,  Win32::Styles::Caption },
			{ IDC_PROP_FORM_SYSMENU,  Win32::Styles::SysMenu },
			{ IDC_PROP_FORM_RESIZABLE, Win32::Styles::ThickFrame },
			{ IDC_PROP_FORM_MINIMIZE, Win32::Styles::MinimizeBox },
			{ IDC_PROP_FORM_MAXIMIZE, Win32::Styles::MaximizeBox },
		};
		for (auto& b : bits)
		{
			bool on = (state.form.style & b.flag) != 0;
			Win32::SendMessageW(Win32::GetDlgItem(panel, b.id),
				Win32::Button::SetCheck,
				on ? Win32::Button::Checked : Win32::Button::Unchecked, 0);
		}

		Win32::SetDlgItemTextW(panel, IDC_PROP_FORM_FONT_LABEL,
			FormFontDisplayString(state.form.font).c_str());

		// Data binding struct name.
		{
			auto bindW = std::wstring(state.form.bindStruct.begin(), state.form.bindStruct.end());
			Win32::SetDlgItemTextW(panel, IDC_PROP_FORM_BINDSTRUCT, bindW.c_str());
		}
	}

	// Refreshes the items displayed in a design-time ComboBox/ListBox HWND.
	void RefreshDesignTimeItems(DesignState& state, ControlEntry& entry)
	{
		auto& ctrl = *entry.control;
		if (ctrl.type != FormDesigner::ControlType::ComboBox &&
			ctrl.type != FormDesigner::ControlType::ListBox)
			return;

		bool isCb = (ctrl.type == FormDesigner::ControlType::ComboBox);
		auto resetMsg = isCb ? Win32::ComboBox::ResetContent : Win32::ListBox::ResetContent;
		auto addMsg   = isCb ? Win32::ComboBox::AddString : Win32::ListBox::AddString;
		auto selMsg   = isCb ? Win32::ComboBox::SetCurSel : Win32::ListBox::SetCurSel;

		Win32::SendMessageW(entry.hwnd, resetMsg, 0, 0);
		for (auto& item : ctrl.items)
			Win32::SendMessageW(entry.hwnd, addMsg, 0,
				reinterpret_cast<Win32::LPARAM>(item.c_str()));
		if (ctrl.selectedIndex >= 0)
			Win32::SendMessageW(entry.hwnd, selMsg, ctrl.selectedIndex, 0);
	}

	// Shows a modal dialog to edit items (one per line). Returns true if user confirmed.
	bool ShowEditItemsDialog(DesignState& state, std::vector<std::wstring>& items)
	{
		constexpr int DLG_W = 300, DLG_H = 350;
		constexpr int IDC_EDIT = 100, IDC_OK = 101, IDC_CANCEL = 102;

		struct DlgData
		{
			std::vector<std::wstring>* items;
			bool confirmed = false;
		};
		DlgData data{ &items };

		// Register a temporary window class for the dialog.
		auto hInst = state.hInstance;
		Win32::WNDCLASSEXW wc = {
			.cbSize = sizeof(Win32::WNDCLASSEXW),
			.lpfnWndProc = Win32::DefWindowProcW,
			.hInstance = hInst,
			.hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
			.hbrBackground = reinterpret_cast<Win32::HBRUSH>(Win32::ColorBtnFace + 1),
			.lpszClassName = L"EditItemsDlg",
		};
		Win32::RegisterClassExW(&wc);

		// Center on parent.
		Win32::RECT parentRc;
		Win32::GetWindowRect(state.surfaceHwnd, &parentRc);
		int cx = (parentRc.left + parentRc.right - DLG_W) / 2;
		int cy = (parentRc.top + parentRc.bottom - DLG_H) / 2;

		auto dlg = Win32::CreateWindowExW(
			Win32::ExStyles::ClientEdge, L"EditItemsDlg", L"Edit Items",
			Win32::Styles::Overlapped | Win32::Styles::Caption | Win32::Styles::SysMenu,
			cx, cy, DLG_W, DLG_H,
			state.surfaceHwnd, nullptr, hInst, nullptr);

		auto font = reinterpret_cast<Win32::WPARAM>(
			Win32::GetStockObject(Win32::DefaultGuiFont));

		// Build initial text: one item per line.
		std::wstring initText;
		for (size_t i = 0; i < items.size(); ++i)
		{
			initText += items[i];
			if (i + 1 < items.size())
				initText += L"\r\n";
		}

		auto edit = Win32::CreateWindowExW(
			Win32::ExStyles::ClientEdge, L"EDIT", initText.c_str(),
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::TabStop |
			Win32::Styles::EditMultiLine | Win32::Styles::EditAutoVScroll |
			Win32::Styles::EditWantReturn | Win32::Styles::VScroll,
			10, 10, DLG_W - 35, DLG_H - 90,
			dlg, reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(IDC_EDIT)),
			hInst, nullptr);
		Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);

		auto okBtn = Win32::CreateWindowExW(0, L"BUTTON", L"OK",
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
			DLG_W - 180, DLG_H - 70, 75, 28,
			dlg, reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(IDC_OK)),
			hInst, nullptr);
		Win32::SendMessageW(okBtn, Win32::Messages::SetFont, font, true);

		auto cancelBtn = Win32::CreateWindowExW(0, L"BUTTON", L"Cancel",
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
			DLG_W - 95, DLG_H - 70, 75, 28,
			dlg, reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(IDC_CANCEL)),
			hInst, nullptr);
		Win32::SendMessageW(cancelBtn, Win32::Messages::SetFont, font, true);

		Win32::ShowWindow(dlg, Win32::Sw_Show);
		Win32::EnableWindow(state.surfaceHwnd, false);

		// Modal message loop.
		Win32::MSG msg;
		bool running = true;
		while (running && Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			if (msg.message == Win32::Messages::Command)
			{
				auto cmdId = Win32::GetLowWord(msg.wParam);
				if (cmdId == IDC_OK && msg.hwnd == dlg)
				{
					// Read the edit text and split by lines.
					int len = Win32::GetWindowTextLengthW(edit);
					auto buf = std::wstring(len + 1, L'\0');
					Win32::GetWindowTextW(edit, buf.data(), len + 1);
					buf.resize(len);

					items.clear();
					std::wistringstream stream(buf);
					std::wstring line;
					while (std::getline(stream, line))
					{
						// Remove trailing \r from \r\n pairs.
						if (!line.empty() && line.back() == L'\r')
							line.pop_back();
						if (!line.empty())
							items.push_back(std::move(line));
					}
					data.confirmed = true;
					running = false;
				}
				else if (cmdId == IDC_CANCEL && msg.hwnd == dlg)
				{
					running = false;
				}
			}
			else if (msg.message == Win32::Messages::Close && msg.hwnd == dlg)
			{
				running = false;
			}
			else
			{
				Win32::TranslateMessage(&msg);
				Win32::DispatchMessageW(&msg);
			}
		}

		Win32::EnableWindow(state.surfaceHwnd, true);
		Win32::SetForegroundWindow(state.surfaceHwnd);
		Win32::DestroyWindow(dlg);
		Win32::UnregisterClassW(L"EditItemsDlg", hInst);

		return data.confirmed;
	}

	export void UpdatePropertyPanel(DesignState& state)
	{
		if (!state.propertyHwnd) return;
		state.updatingProperties = true;
		state.invalidFields.clear();

		auto panel = state.propertyHwnd;
		int sel = SingleSelection(state);
		bool hasSel = sel >= 0 && sel < static_cast<int>(state.entries.size());

		UpdatePropertyVisibility(state, panel, hasSel);
		ResetPropertyScroll(state);
		UpdateScrollRange(state);

		if (hasSel)
			UpdateControlProperties(state, panel, sel);
		else
			UpdateFormProperties(state, panel);

		state.updatingProperties = false;
		UpdateStatusBar(state);
	}

	void ReadEventHandler(Win32::HWND panel, Win32::UINT controlId, std::string& target)
	{
		wchar_t buf[256] = {};
		Win32::GetDlgItemTextW(panel, controlId, buf, 256);
		target = NarrowString(buf, std::wcslen(buf));
	}

	void ApplyPropertyChange(DesignState& state, Win32::UINT controlId)
	{
		int sel = SingleSelection(state);
		if (sel < 0 || sel >= static_cast<int>(state.entries.size()))
			return;

		PushUndo(state);

		auto& entry = state.entries[sel];
		auto& ctrl = *entry.control;
		auto panel = state.propertyHwnd;

		switch (controlId)
		{
		case IDC_PROP_TEXT:
		{
			wchar_t buf[512] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_TEXT, buf, 512);
			ctrl.text = buf;
			Win32::SetWindowTextW(entry.hwnd, ctrl.text.c_str());
			break;
		}
		case IDC_PROP_ID:
		{
			Win32::BOOL ok = false;
			auto val = Win32::GetDlgItemInt(panel, IDC_PROP_ID, &ok, false);
			if (ok) ctrl.id = static_cast<int>(val);
			break;
		}
		case IDC_PROP_X:
		case IDC_PROP_Y:
		case IDC_PROP_W:
		case IDC_PROP_H:
		{
			Win32::BOOL ok = false;
			bool isSigned = (controlId == IDC_PROP_X || controlId == IDC_PROP_Y);
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, controlId, &ok, isSigned));
			if (ok)
			{
				switch (controlId)
				{
				case IDC_PROP_X: ctrl.rect.x = val; break;
				case IDC_PROP_Y: ctrl.rect.y = val; break;
				case IDC_PROP_W: if (val >= BASE_MIN_CONTROL_SIZE) ctrl.rect.width = val; break;
				case IDC_PROP_H: if (val >= BASE_MIN_CONTROL_SIZE) ctrl.rect.height = val; break;
				}
			}
			int ro = RulerOffset(state);
			Win32::MoveWindow(entry.hwnd, ctrl.rect.x + ro, ctrl.rect.y + ro,
				ctrl.rect.width, ctrl.rect.height, true);
			Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
			break;
		}
		case IDC_PROP_ONCLICK:     ReadEventHandler(panel, controlId, ctrl.onClick); break;
		case IDC_PROP_ONCHANGE:    ReadEventHandler(panel, controlId, ctrl.onChange); break;
		case IDC_PROP_ONDBLCLICK:  ReadEventHandler(panel, controlId, ctrl.onDoubleClick); break;
		case IDC_PROP_ONSELCHANGE: ReadEventHandler(panel, controlId, ctrl.onSelectionChange); break;
		case IDC_PROP_ONFOCUS:     ReadEventHandler(panel, controlId, ctrl.onFocus); break;
		case IDC_PROP_ONBLUR:      ReadEventHandler(panel, controlId, ctrl.onBlur); break;
		case IDC_PROP_ONCHECK:     ReadEventHandler(panel, controlId, ctrl.onCheck); break;
		case IDC_PROP_TABINDEX:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_TABINDEX, &ok, true));
			if (ok) ctrl.tabIndex = val;
			break;
		}
		case IDC_PROP_TEXTALIGN:
		{
			auto combo = Win32::GetDlgItem(panel, IDC_PROP_TEXTALIGN);
			auto sel = static_cast<int>(Win32::SendMessageW(combo, Win32::ComboBox::GetCurSel, 0, 0));
			if (sel >= 0 && sel <= 2)
			{
				ctrl.textAlign = static_cast<FormDesigner::TextAlign>(sel);
				RebuildSingleControl(state, entry);
			}
			break;
		}
		case IDC_PROP_ANCHOR:
		{
			using namespace FormDesigner::Anchor;
			constexpr int anchorValues[] = {
				Top | Left, Top | Right, Bottom | Left, Bottom | Right,
				Top | Left | Right, Bottom | Left | Right,
				Top | Bottom | Left, Top | Bottom | Right,
				Top | Bottom | Left | Right,
			};
			auto combo = Win32::GetDlgItem(panel, IDC_PROP_ANCHOR);
			auto sel = static_cast<int>(Win32::SendMessageW(combo, Win32::ComboBox::GetCurSel, 0, 0));
			if (sel >= 0 && sel < 9)
				ctrl.anchor = anchorValues[sel];
			break;
		}
		case IDC_PROP_TOOLTIP:
		{
			wchar_t buf[256] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_TOOLTIP, buf, 256);
			ctrl.tooltip = buf;
			break;
		}
		case IDC_PROP_SELINDEX:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_SELINDEX, &ok, true));
			if (ok)
			{
				int maxIdx = static_cast<int>(ctrl.items.size()) - 1;
				ctrl.selectedIndex = (val < 0) ? -1 : std::min(val, maxIdx);
				RefreshDesignTimeItems(state, entry);
			}
			break;
		}
		case IDC_PROP_VAL_MINLEN:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_VAL_MINLEN, &ok, false));
			if (ok) ctrl.validation.minLength = val;
			break;
		}
		case IDC_PROP_VAL_MAXLEN:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_VAL_MAXLEN, &ok, false));
			if (ok) ctrl.validation.maxLength = val;
			break;
		}
		case IDC_PROP_VAL_PATTERN:
		{
			wchar_t buf[256] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_VAL_PATTERN, buf, 256);
			auto wide = std::wstring(buf);
			ctrl.validation.pattern = std::string(wide.begin(), wide.end());
			break;
		}
		case IDC_PROP_VAL_MIN:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_VAL_MIN, &ok, true));
			if (ok) ctrl.validation.min = val;
			break;
		}
		case IDC_PROP_VAL_MAX:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_VAL_MAX, &ok, true));
			if (ok) ctrl.validation.max = val;
			break;
		}
		case IDC_PROP_VALUE:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_VALUE, &ok, true));
			if (ok) ctrl.value = val;
			break;
		}
		case IDC_PROP_IMAGEPATH:
		{
			wchar_t buf[Win32::MaxPath] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_IMAGEPATH, buf, Win32::MaxPath);
			ctrl.imagePath = buf;
			// Update designer preview text.
			Win32::SetWindowTextW(entry.hwnd,
				ctrl.imagePath.empty() ? L"[Picture]" : ctrl.imagePath.c_str());
			Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
			break;
		}
		case IDC_PROP_BINDFIELD:
		{
			wchar_t buf[256] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_BINDFIELD, buf, 256);
			auto narrow = std::string{};
			for (auto* p = buf; *p; ++p) narrow += static_cast<char>(*p);
			ctrl.bindField = narrow;
			break;
		}
		case IDC_PROP_ACCESSNAME:
		{
			wchar_t buf[256] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_ACCESSNAME, buf, 256);
			ctrl.accessibleName = buf;
			break;
		}
		case IDC_PROP_ACCESSDESC:
		{
			wchar_t buf[256] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_ACCESSDESC, buf, 256);
			ctrl.accessibleDescription = buf;
			break;
		}
		default:
			return;
		}

		MarkDirty(state);
	}

	void ApplyFormPropertyChange(DesignState& state, Win32::UINT controlId)
	{
		PushUndo(state);
		auto panel = state.propertyHwnd;

		switch (controlId)
		{
		case IDC_PROP_FORM_TITLE:
		{
			wchar_t buf[256] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_FORM_TITLE, buf, 256);
			state.form.title = buf;
			break;
		}
		case IDC_PROP_FORM_WIDTH:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_FORM_WIDTH, &ok, false));
			if (ok && val >= 50) state.form.width = val;
			break;
		}
		case IDC_PROP_FORM_HEIGHT:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, IDC_PROP_FORM_HEIGHT, &ok, false));
			if (ok && val >= 50) state.form.height = val;
			break;
		}
		case IDC_PROP_FORM_BGCOLOR:
		{
			wchar_t buf[16] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_FORM_BGCOLOR, buf, 16);
			auto hex = std::wstring(buf);
			state.form.backgroundColor = hex.empty() ? -1 : HexToColorRef(hex);
			Win32::InvalidateRect(state.canvasHwnd, nullptr, true);
			break;
		}
		case IDC_PROP_FORM_BINDSTRUCT:
		{
			wchar_t buf[256] = {};
			Win32::GetDlgItemTextW(panel, IDC_PROP_FORM_BINDSTRUCT, buf, 256);
			auto narrow = std::string{};
			for (auto* p = buf; *p; ++p) narrow += static_cast<char>(*p);
			state.form.bindStruct = narrow;
			break;
		}
		default:
			return;
		}

		MarkDirty(state);
	}

	// Validates a property field and returns an error message (empty = valid).
	auto ValidateField(DesignState& state, Win32::UINT id) -> std::wstring
	{
		auto panel = state.propertyHwnd;

		switch (id)
		{
		case IDC_PROP_W:
		case IDC_PROP_H:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, id, &ok, false));
			if (!ok) return L"Must be a number";
			if (val < BASE_MIN_CONTROL_SIZE)
				return std::format(L"Must be >= {}", BASE_MIN_CONTROL_SIZE);
			break;
		}
		case IDC_PROP_X:
		case IDC_PROP_Y:
		{
			Win32::BOOL ok = false;
			Win32::GetDlgItemInt(panel, id, &ok, true);
			if (!ok) return L"Must be a number";
			break;
		}
		case IDC_PROP_ID:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, id, &ok, false));
			if (!ok) return L"Must be a number";
			int sel = SingleSelection(state);
			if (sel >= 0 && IsDuplicateId(state, val, sel))
				return L"Duplicate control ID";
			break;
		}
		case IDC_PROP_TABINDEX:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, id, &ok, true));
			if (!ok) return L"Must be a number";
			if (val < 0) return L"Must be >= 0";
			break;
		}
		case IDC_PROP_SELINDEX:
		{
			Win32::BOOL ok = false;
			Win32::GetDlgItemInt(panel, id, &ok, true);
			if (!ok) return L"Must be a number";
			break;
		}
		case IDC_PROP_ONCLICK:
		case IDC_PROP_ONCHANGE:
		case IDC_PROP_ONDBLCLICK:
		case IDC_PROP_ONSELCHANGE:
		case IDC_PROP_ONFOCUS:
		case IDC_PROP_ONBLUR:
		case IDC_PROP_ONCHECK:
		{
			wchar_t buf[256] = {};
			Win32::GetDlgItemTextW(panel, id, buf, 256);
			if (!IsValidIdentifier(buf))
				return L"Must be a valid C++ identifier";
			break;
		}
		case IDC_PROP_FORM_WIDTH:
		case IDC_PROP_FORM_HEIGHT:
		{
			Win32::BOOL ok = false;
			auto val = static_cast<int>(Win32::GetDlgItemInt(panel, id, &ok, false));
			if (!ok) return L"Must be a number";
			if (val < 50) return L"Must be >= 50";
			break;
		}
		}
		return {};
	}

	void ShowValidationError(DesignState& state, const std::wstring& msg)
	{
		if (state.statusbarHwnd)
			Win32::SendMessageW(state.statusbarHwnd, Win32::StatusBar::SetTextW, 0,
				reinterpret_cast<Win32::LPARAM>(msg.c_str()));
	}

	export void CreatePropertyControls(DesignState& state)
	{
		auto parent = state.propertyHwnd;
		auto hInst = state.hInstance;
		auto font = reinterpret_cast<Win32::WPARAM>(
			Win32::GetStockObject(Win32::DefaultGuiFont));

		// DPI-scaled layout constants.
		auto& d = state.dpiInfo;
		int pad    = d.Scale(5);
		int lw     = d.Scale(55);   // label width
		int lh     = d.Scale(18);   // label height
		int ex     = d.Scale(65);   // edit x
		int ew     = d.Scale(150);  // edit width
		int ch     = d.Scale(22);   // control height
		int rh     = d.Scale(26);   // row height
		int hdrH   = d.Scale(20);   // header height
		int startY = d.Scale(30);   // first row y

		auto header = Win32::CreateWindowExW(0, L"STATIC", L"Properties",
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticLeft,
			pad, pad, d.Scale(210), hdrH, parent, nullptr, hInst, nullptr);
		Win32::SendMessageW(header, Win32::Messages::SetFont, font, true);

		// Control property rows (visible when a control is selected).
		struct PropRow { const wchar_t* label; Win32::UINT editId; Win32::DWORD extraStyle; };

		PropRow ctrlRows[] = {
			{ L"Type:",    IDC_PROP_TYPE,    Win32::Styles::EditReadOnly },
			{ L"Text:",    IDC_PROP_TEXT,    Win32::Styles::EditAutoHScroll },
			{ L"ID:",      IDC_PROP_ID,     Win32::Styles::EditAutoHScroll },
			{ L"X:",       IDC_PROP_X,      Win32::Styles::EditAutoHScroll },
			{ L"Y:",       IDC_PROP_Y,      Win32::Styles::EditAutoHScroll },
			{ L"Width:",   IDC_PROP_W,      Win32::Styles::EditAutoHScroll },
			{ L"Height:",  IDC_PROP_H,      Win32::Styles::EditAutoHScroll },
			{ L"onClick:",  IDC_PROP_ONCLICK,     Win32::Styles::EditAutoHScroll },
			{ L"onChange:",  IDC_PROP_ONCHANGE,    Win32::Styles::EditAutoHScroll },
			{ L"onDblClk:", IDC_PROP_ONDBLCLICK,  Win32::Styles::EditAutoHScroll },
			{ L"onSelChg:", IDC_PROP_ONSELCHANGE, Win32::Styles::EditAutoHScroll },
			{ L"onFocus:", IDC_PROP_ONFOCUS,     Win32::Styles::EditAutoHScroll },
			{ L"onBlur:",  IDC_PROP_ONBLUR,      Win32::Styles::EditAutoHScroll },
			{ L"onCheck:", IDC_PROP_ONCHECK,     Win32::Styles::EditAutoHScroll },
			{ L"TabIdx:", IDC_PROP_TABINDEX,    Win32::Styles::EditAutoHScroll },
			{ L"Tooltip:", IDC_PROP_TOOLTIP,    Win32::Styles::EditAutoHScroll },
		};

		int y = startY;
		for (auto& row : ctrlRows)
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", row.label,
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::TabStop | row.extraStyle,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			Win32::EnableWindow(edit, false);

			y += rh;
		}

		// Text alignment dropdown (ComboBox instead of EDIT).
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Align:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_TEXTALIGN + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto combo = Win32::CreateWindowExW(0, Win32::Controls::ComboBox, nullptr,
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::ComboBoxDropDownList,
				ex, y, ew, d.Scale(120), parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_TEXTALIGN)),
				hInst, nullptr);
			Win32::SendMessageW(combo, Win32::Messages::SetFont, font, true);
			Win32::SendMessageW(combo, Win32::ComboBox::AddString, 0,
				reinterpret_cast<Win32::LPARAM>(L"Left"));
			Win32::SendMessageW(combo, Win32::ComboBox::AddString, 0,
				reinterpret_cast<Win32::LPARAM>(L"Center"));
			Win32::SendMessageW(combo, Win32::ComboBox::AddString, 0,
				reinterpret_cast<Win32::LPARAM>(L"Right"));
			Win32::SendMessageW(combo, Win32::ComboBox::SetCurSel, 0, 0);
		}

		// Locked checkbox (visible when a control is selected).
		y += rh;
		{
			auto chk = Win32::CreateWindowExW(0, Win32::Controls::Button, L"Locked",
				Win32::Styles::Child | Win32::Styles::AutoCheckBox,
				d.Scale(15), y, d.Scale(90), hdrH, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_LOCKED)),
				hInst, nullptr);
			Win32::SendMessageW(chk, Win32::Messages::SetFont, font, true);
		}

		// Visible checkbox (next to Locked on the same row).
		{
			auto chk = Win32::CreateWindowExW(0, Win32::Controls::Button, L"Visible",
				Win32::Styles::Child | Win32::Styles::AutoCheckBox,
				d.Scale(110), y, d.Scale(90), hdrH, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_VISIBLE)),
				hInst, nullptr);
			Win32::SendMessageW(chk, Win32::Messages::SetFont, font, true);
		}

		// Enabled checkbox (below Locked/Visible row).
		y += d.Scale(24);
		{
			auto chk = Win32::CreateWindowExW(0, Win32::Controls::Button, L"Enabled",
				Win32::Styles::Child | Win32::Styles::AutoCheckBox,
				d.Scale(15), y, d.Scale(90), hdrH, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ENABLED)),
				hInst, nullptr);
			Win32::SendMessageW(chk, Win32::Messages::SetFont, font, true);
		}

		// Anchor dropdown.
		y += d.Scale(24);
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Anchor:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ANCHOR + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto combo = Win32::CreateWindowExW(0, Win32::Controls::ComboBox, nullptr,
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::ComboBoxDropDownList,
				ex, y, ew, d.Scale(200), parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ANCHOR)),
				hInst, nullptr);
			Win32::SendMessageW(combo, Win32::Messages::SetFont, font, true);
			const wchar_t* anchorItems[] = {
				L"Top, Left",
				L"Top, Right",
				L"Bottom, Left",
				L"Bottom, Right",
				L"Top, Left, Right",
				L"Bottom, Left, Right",
				L"Top, Bottom, Left",
				L"Top, Bottom, Right",
				L"All",
			};
			for (auto* item : anchorItems)
				Win32::SendMessageW(combo, Win32::ComboBox::AddString, 0,
					reinterpret_cast<Win32::LPARAM>(item));
			Win32::SendMessageW(combo, Win32::ComboBox::SetCurSel, 0, 0);
		}

		// Control font row: label + "..." button + "Clear" button.
		y += rh;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Font:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FONT_LABEL + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto fontLabel = Win32::CreateWindowExW(0, L"STATIC", L"(inherited)",
				Win32::Styles::Child | Win32::Styles::StaticLeft,
				ex, y + 2, d.Scale(85), lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FONT_LABEL)),
				hInst, nullptr);
			Win32::SendMessageW(fontLabel, Win32::Messages::SetFont, font, true);

			auto fontBtn = Win32::CreateWindowExW(0, L"BUTTON", L"...",
				Win32::Styles::Child | Win32::Styles::ButtonPush,
				d.Scale(155), y, d.Scale(30), ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FONT_BTN)),
				hInst, nullptr);
			Win32::SendMessageW(fontBtn, Win32::Messages::SetFont, font, true);

			auto fontClear = Win32::CreateWindowExW(0, L"BUTTON", L"X",
				Win32::Styles::Child | Win32::Styles::ButtonPush,
				d.Scale(190), y, d.Scale(25), ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FONT_CLEAR)),
				hInst, nullptr);
			Win32::SendMessageW(fontClear, Win32::Messages::SetFont, font, true);
		}

		// Bind field row (control-level data binding).
		y += rh;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Bind:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_BINDFIELD + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::EditAutoHScroll,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_BINDFIELD)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			Win32::EnableWindow(edit, false);
		}

		// Accessibility: AccessName row.
		y += rh;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"AccName:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ACCESSNAME + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::EditAutoHScroll,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ACCESSNAME)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			Win32::EnableWindow(edit, false);
		}

		// Accessibility: AccessDesc row.
		y += rh;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"AccDesc:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ACCESSDESC + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::EditAutoHScroll,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ACCESSDESC)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			Win32::EnableWindow(edit, false);
		}

		// Accessibility: TabStop checkbox.
		y += rh;
		{
			auto chk = Win32::CreateWindowExW(0, Win32::Controls::Button, L"Tab Stop",
				Win32::Styles::Child | Win32::Styles::AutoCheckBox,
				d.Scale(15), y, d.Scale(95), hdrH, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_TABSTOP)),
				hInst, nullptr);
			Win32::SendMessageW(chk, Win32::Messages::SetFont, font, true);

			auto chk2 = Win32::CreateWindowExW(0, Win32::Controls::Button, L"Group Start",
				Win32::Styles::Child | Win32::Styles::AutoCheckBox,
				d.Scale(115), y, d.Scale(100), hdrH, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_GROUPSTART)),
				hInst, nullptr);
			Win32::SendMessageW(chk2, Win32::Messages::SetFont, font, true);
		}

		// Image path row (Picture only): label + edit + "..." browse button.
		// Shares the same y slot as Items — they are mutually exclusive.
		y += rh;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Image:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_IMAGEPATH + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::EditAutoHScroll,
				ex, y, d.Scale(105), ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_IMAGEPATH)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);

			auto btn = Win32::CreateWindowExW(0, L"BUTTON", L"...",
				Win32::Styles::Child | Win32::Styles::ButtonPush,
				d.Scale(175), y, d.Scale(40), ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_IMAGEPATH_BTN)),
				hInst, nullptr);
			Win32::SendMessageW(btn, Win32::Messages::SetFont, font, true);
		}

		// Items row: read-only label + "Edit..." button (ComboBox/ListBox only).
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Items:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ITEMS_LABEL + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto itemsLabel = Win32::CreateWindowExW(0, L"STATIC", L"0 items",
				Win32::Styles::Child | Win32::Styles::StaticLeft,
				ex, y + 2, d.Scale(85), lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ITEMS_LABEL)),
				hInst, nullptr);
			Win32::SendMessageW(itemsLabel, Win32::Messages::SetFont, font, true);

			auto itemsBtn = Win32::CreateWindowExW(0, L"BUTTON", L"Edit...",
				Win32::Styles::Child | Win32::Styles::ButtonPush,
				d.Scale(155), y, d.Scale(60), ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ITEMS_BTN)),
				hInst, nullptr);
			Win32::SendMessageW(itemsBtn, Win32::Messages::SetFont, font, true);
		}

		// SelectedIndex row (ComboBox/ListBox only).
		y += rh;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"SelIdx:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_SELINDEX + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"-1",
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::EditAutoHScroll,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_SELINDEX)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			Win32::EnableWindow(edit, false);
		}

		// Validation section header.
		y += rh;
		{
			auto hdr = Win32::CreateWindowExW(0, L"STATIC", L"Validation",
				Win32::Styles::Child,
				pad, y + 2, d.Scale(210), lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_VAL_REQUIRED + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(hdr, Win32::Messages::SetFont, font, true);
		}
		y += ch;

		// Required checkbox.
		{
			auto chk = Win32::CreateWindowExW(0, Win32::Controls::Button, L"Required",
				Win32::Styles::Child | Win32::Styles::AutoCheckBox,
				d.Scale(15), y, d.Scale(200), hdrH, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_VAL_REQUIRED)),
				hInst, nullptr);
			Win32::SendMessageW(chk, Win32::Messages::SetFont, font, true);
		}
		y += ch;

		// MinLength / MaxLength / Pattern (text validation).
		PropRow textValRows[] = {
			{ L"MinLen:", IDC_PROP_VAL_MINLEN, Win32::Styles::EditAutoHScroll },
			{ L"MaxLen:", IDC_PROP_VAL_MAXLEN, Win32::Styles::EditAutoHScroll },
			{ L"Pattern:", IDC_PROP_VAL_PATTERN, Win32::Styles::EditAutoHScroll },
		};
		for (auto& row : textValRows)
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", row.label,
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::TabStop | row.extraStyle,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			y += rh;
		}

		// Min / Max (range validation).
		PropRow rangeValRows[] = {
			{ L"Min:", IDC_PROP_VAL_MIN, Win32::Styles::EditAutoHScroll },
			{ L"Max:", IDC_PROP_VAL_MAX, Win32::Styles::EditAutoHScroll },
		};
		for (auto& row : rangeValRows)
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", row.label,
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::TabStop | row.extraStyle,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			y += rh;
		}

		// Value (ProgressBar, TrackBar, UpDown).
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Value:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_VALUE + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"0",
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::EditAutoHScroll,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_VALUE)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			y += rh;
		}

		// Form property rows (visible when no control is selected).
		PropRow formRows[] = {
			{ L"Title:",   IDC_PROP_FORM_TITLE,  Win32::Styles::EditAutoHScroll },
			{ L"Width:",   IDC_PROP_FORM_WIDTH,  Win32::Styles::EditAutoHScroll },
			{ L"Height:",  IDC_PROP_FORM_HEIGHT, Win32::Styles::EditAutoHScroll },
			{ L"BgColor:", IDC_PROP_FORM_BGCOLOR, Win32::Styles::EditAutoHScroll },
		};

		y = startY;
		for (auto& row : formRows)
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", row.label,
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			int editW = (row.editId == IDC_PROP_FORM_BGCOLOR) ? d.Scale(105) : ew;
			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::TabStop | row.extraStyle,
				ex, y, editW, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);

			y += rh;
		}

		// Color picker button next to BgColor.
		auto bgBtn = Win32::CreateWindowExW(0, L"BUTTON", L"...",
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
			d.Scale(175), startY + 3 * rh, d.Scale(40), ch, parent,
			reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_BGCOLOR_BTN)),
			hInst, nullptr);
		Win32::SendMessageW(bgBtn, Win32::Messages::SetFont, font, true);

		// Window style checkboxes (visible when no control is selected).
		struct StyleCheck { const wchar_t* label; Win32::UINT id; };
		StyleCheck styleChecks[] = {
			{ L"Title Bar",      IDC_PROP_FORM_CAPTION },
			{ L"System Menu",    IDC_PROP_FORM_SYSMENU },
			{ L"Resizable",      IDC_PROP_FORM_RESIZABLE },
			{ L"Minimize Box",   IDC_PROP_FORM_MINIMIZE },
			{ L"Maximize Box",   IDC_PROP_FORM_MAXIMIZE },
		};

		y = startY + 4 * rh + d.Scale(10); // After the 4 form property edit rows + spacing.
		auto styleHeader = Win32::CreateWindowExW(0, L"STATIC", L"Window Style",
			Win32::Styles::Child | Win32::Styles::Visible,
			pad, y, d.Scale(210), lh, parent,
			reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_CAPTION + IDL_OFFSET)),
			hInst, nullptr);
		Win32::SendMessageW(styleHeader, Win32::Messages::SetFont, font, true);
		y += ch;

		for (auto& sc : styleChecks)
		{
			auto chk = Win32::CreateWindowExW(0, Win32::Controls::Button, sc.label,
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::AutoCheckBox,
				d.Scale(15), y, d.Scale(200), hdrH, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(sc.id)),
				hInst, nullptr);
			Win32::SendMessageW(chk, Win32::Messages::SetFont, font, true);
			y += ch;
		}

		// Form font row: label + "..." button + "Clear" button.
		y += d.Scale(8);
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Font:",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_FONT_LABEL + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto fontLabel = Win32::CreateWindowExW(0, L"STATIC", L"(default)",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticLeft,
				ex, y + 2, d.Scale(85), lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_FONT_LABEL)),
				hInst, nullptr);
			Win32::SendMessageW(fontLabel, Win32::Messages::SetFont, font, true);

			auto fontBtn = Win32::CreateWindowExW(0, L"BUTTON", L"...",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
				d.Scale(155), y, d.Scale(30), ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_FONT_BTN)),
				hInst, nullptr);
			Win32::SendMessageW(fontBtn, Win32::Messages::SetFont, font, true);

			auto fontClear = Win32::CreateWindowExW(0, L"BUTTON", L"X",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
				d.Scale(190), y, d.Scale(25), ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_FONT_CLEAR)),
				hInst, nullptr);
			Win32::SendMessageW(fontClear, Win32::Messages::SetFont, font, true);
		}

		// Form bind struct row.
		y += rh;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Bind:",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticRight,
				pad, y + 2, lw, lh, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_BINDSTRUCT + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::TabStop | Win32::Styles::EditAutoHScroll,
				ex, y, ew, ch, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_BINDSTRUCT)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
		}

		// Create tooltip window for property panel buttons.
		state.propTooltipHwnd = FormDesigner::CreateTooltipWindow(parent, hInst);
		if (state.propTooltipHwnd)
		{
			struct BtnTip { Win32::UINT id; const wchar_t* text; };
			BtnTip tips[] = {
				{ IDC_PROP_FONT_BTN,          L"Choose control font" },
				{ IDC_PROP_FONT_CLEAR,        L"Clear custom font (use inherited)" },
				{ IDC_PROP_ITEMS_BTN,         L"Edit items (one per line)" },
				{ IDC_PROP_IMAGEPATH_BTN,     L"Browse for image file (BMP/ICO)" },
				{ IDC_PROP_FORM_BGCOLOR_BTN,  L"Choose background color" },
				{ IDC_PROP_FORM_FONT_BTN,     L"Choose form font" },
				{ IDC_PROP_FORM_FONT_CLEAR,   L"Clear form font (use default)" },
			};
			for (auto& bt : tips)
			{
				auto hwnd = Win32::GetDlgItem(parent, bt.id);
				if (!hwnd) continue;
				Win32::TTTOOLINFOW ti = {};
				ti.cbSize = sizeof(Win32::TTTOOLINFOW);
				ti.uFlags = Win32::TooltipFlags::Subclass | Win32::TooltipFlags::IdIsHwnd;
				ti.hwnd = parent;
				ti.uId = reinterpret_cast<Win32::UINT_PTR>(hwnd);
				ti.lpszText = const_cast<wchar_t*>(bt.text);
				Win32::SendMessageW(state.propTooltipHwnd,
					Win32::TooltipMessages::AddTool, 0,
					reinterpret_cast<Win32::LPARAM>(&ti));
			}
		}
	}

	auto PropContentCtrl(const DpiInfo& d) -> int {
		// header(30) + 31 rows*rh(26) + locked/visible(24) + enabled(24) + tabstop/group(24) + val header+required(22*2) + padding(10)
		return d.Scale(30) + 31 * d.Scale(26) + 3 * d.Scale(24) + 2 * d.Scale(22) + d.Scale(10);
	}
	auto PropContentForm(const DpiInfo& d) -> int { return d.Scale(30) + 4 * d.Scale(26) + d.Scale(10) + d.Scale(22) + 5 * d.Scale(22) + d.Scale(8) + d.Scale(26) + d.Scale(26) + d.Scale(10); }
	auto PropScrollLine(const DpiInfo& d) -> int { return d.Scale(26); }

	void UpdateScrollRange(DesignState& state)
	{
		auto panel = state.propertyHwnd;
		int sel = SingleSelection(state);
		bool hasSel = sel >= 0 && sel < static_cast<int>(state.entries.size());
		int contentHeight = hasSel ? PropContentCtrl(state.dpiInfo) : PropContentForm(state.dpiInfo);

		Win32::RECT rc;
		Win32::GetClientRect(panel, &rc);

		Win32::SCROLLINFO si = {
			.cbSize = sizeof(Win32::SCROLLINFO),
			.fMask = Win32::ScrollInfo::Range | Win32::ScrollInfo::Page,
			.nMin = 0,
			.nMax = contentHeight,
			.nPage = static_cast<Win32::UINT>(rc.bottom),
		};
		Win32::SetScrollInfo(panel, Win32::ScrollBar::Vert, &si, true);
	}

	void ScrollPropertyPanel(DesignState& state, int newPos)
	{
		auto panel = state.propertyHwnd;

		// Clamp.
		Win32::SCROLLINFO si = { .cbSize = sizeof(Win32::SCROLLINFO), .fMask = Win32::ScrollInfo::All };
		Win32::GetScrollInfo(panel, Win32::ScrollBar::Vert, &si);
		int maxPos = si.nMax - static_cast<int>(si.nPage);
		if (maxPos < 0) maxPos = 0;
		if (newPos < 0) newPos = 0;
		if (newPos > maxPos) newPos = maxPos;

		int delta = state.propertyScrollY - newPos;
		if (delta == 0) return;

		state.propertyScrollY = newPos;
		si.fMask = Win32::ScrollInfo::Pos;
		si.nPos = newPos;
		Win32::SetScrollInfo(panel, Win32::ScrollBar::Vert, &si, true);
		Win32::ScrollWindowEx(panel, 0, delta, nullptr, nullptr, nullptr, nullptr,
			Win32::ScrollWindow::ScrollChildren | Win32::ScrollWindow::Invalidate |
			Win32::ScrollWindow::Erase);
	}

	void ResetPropertyScroll(DesignState& state)
	{
		if (state.propertyScrollY != 0)
			ScrollPropertyPanel(state, 0);
	}

	Win32::COLORREF g_customColors[16] = {};

	// Shows the Win32 ChooseFont dialog pre-populated with the given font info.
	// Returns true and populates result if the user confirmed; false on cancel.
	bool ShowFontDialog(DesignState& state, const FormDesigner::FontInfo& resolved,
		FormDesigner::FontInfo& result)
	{
		Win32::LOGFONTW lf = {};
		auto hdc = Win32::GetDC(state.surfaceHwnd);
		lf.lfHeight = -Win32::MulDiv(resolved.size, Win32::GetDeviceCaps(
			hdc, Win32::FontMetrics::LogPixelsY), 72);
		Win32::ReleaseDC(state.surfaceHwnd, hdc);
		lf.lfWeight = resolved.bold ? Win32::FontWeight::Bold : Win32::FontWeight::Normal;
		lf.lfItalic = resolved.italic ? 1 : 0;
		lf.lfCharSet = Win32::FontCharset::Default;
		lf.lfOutPrecision = Win32::FontPrecision::OutDefault;
		lf.lfClipPrecision = Win32::FontPrecision::ClipDefault;
		lf.lfQuality = Win32::FontPrecision::QualityDefault;
		auto len = resolved.family.size();
		if (len > 31) len = 31;
		for (size_t i = 0; i < len; ++i) lf.lfFaceName[i] = resolved.family[i];
		lf.lfFaceName[len] = L'\0';

		Win32::CHOOSEFONTW cf = {};
		cf.lStructSize = sizeof(Win32::CHOOSEFONTW);
		cf.hwndOwner = state.surfaceHwnd;
		cf.lpLogFont = &lf;
		cf.Flags = Win32::FontDialog::ScreenFonts | Win32::FontDialog::InitToLogFont
			| Win32::FontDialog::NoSimulations;
		cf.iPointSize = resolved.size * 10;

		if (!Win32::ChooseFontW(&cf))
			return false;

		result.family = lf.lfFaceName;
		result.size = cf.iPointSize / 10;
		result.bold = (lf.lfWeight >= Win32::FontWeight::Bold);
		result.italic = (lf.lfItalic != 0);
		return true;
	}

	// Validates a property field and applies changes if valid, showing errors otherwise.
	void ValidateAndApply(DesignState& state, Win32::HWND panel, Win32::UINT id, bool isFormProp)
	{
		auto err = ValidateField(state, id);
		if (!err.empty())
		{
			state.invalidFields.insert(id);
			ShowValidationError(state, err);
		}
		else
		{
			state.invalidFields.erase(id);
			ShowValidationError(state, L"");
			if (isFormProp)
				ApplyFormPropertyChange(state, id);
			else
				ApplyPropertyChange(state, id);
		}
		Win32::InvalidateRect(Win32::GetDlgItem(panel, id), nullptr, true);
	}

	export auto PropertyPanelProc(Win32::HWND hwnd, Win32::UINT msg,
		Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
	{
		auto* state = reinterpret_cast<DesignState*>(
			Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

		switch (msg)
		{
		case Win32::Messages::Size:
		{
			if (state) UpdateScrollRange(*state);
			break;
		}

		case Win32::Messages::VScroll:
		{
			if (!state) break;
			Win32::SCROLLINFO si = { .cbSize = sizeof(Win32::SCROLLINFO), .fMask = Win32::ScrollInfo::All };
			Win32::GetScrollInfo(hwnd, Win32::ScrollBar::Vert, &si);
			int pos = state->propertyScrollY;

			int scrollLine = PropScrollLine(state->dpiInfo);
			switch (Win32::GetLowWord(wParam))
			{
			case Win32::ScrollBar::LineUp:        pos -= scrollLine; break;
			case Win32::ScrollBar::LineDown:      pos += scrollLine; break;
			case Win32::ScrollBar::PageUp:        pos -= static_cast<int>(si.nPage); break;
			case Win32::ScrollBar::PageDown:      pos += static_cast<int>(si.nPage); break;
			case Win32::ScrollBar::ThumbTrack:    pos = si.nTrackPos; break;
			case Win32::ScrollBar::ThumbPosition: pos = si.nTrackPos; break;
			case Win32::ScrollBar::Top:           pos = si.nMin; break;
			case Win32::ScrollBar::Bottom:        pos = si.nMax; break;
			}
			ScrollPropertyPanel(*state, pos);
			return 0;
		}

		case Win32::Messages::MouseWheel:
		{
			if (!state) break;
			int delta = Win32::GetWheelDelta(wParam);
			int lines = delta / 120;
			ScrollPropertyPanel(*state, state->propertyScrollY - lines * PropScrollLine(state->dpiInfo));
			return 0;
		}

		case Win32::Messages::Command:
		{
			if (!state || state->updatingProperties) break;

			auto code = Win32::GetHighWord(wParam);
			auto id   = Win32::GetLowWord(wParam);

			// Color picker button.
			if (id == IDC_PROP_FORM_BGCOLOR_BTN && code == Win32::Notifications::ButtonClicked)
			{
				Win32::COLORREF initial = (state->form.backgroundColor != -1)
					? static_cast<Win32::COLORREF>(state->form.backgroundColor)
					: Win32::GetSysColor(Win32::ColorBtnFace);

				Win32::CHOOSECOLORW cc = {
					.lStructSize = sizeof(Win32::CHOOSECOLORW),
					.hwndOwner = state->surfaceHwnd,
					.rgbResult = initial,
					.lpCustColors = g_customColors,
					.Flags = Win32::ColorDialog::FullOpen | Win32::ColorDialog::RgbInit,
				};

				if (Win32::ChooseColorW(&cc))
				{
					PushUndo(*state);
					state->form.backgroundColor = static_cast<int>(cc.rgbResult);
					UpdatePropertyPanel(*state);
					Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
					MarkDirty(*state);
				}
				return 0;
			}

			// Locked checkbox for selected control.
			if (id == IDC_PROP_LOCKED && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;
				PushUndo(*state);
				auto chk = Win32::GetDlgItem(hwnd, IDC_PROP_LOCKED);
				state->entries[sel].control->locked =
					Win32::SendMessageW(chk, Win32::Button::GetCheck, 0, 0) == Win32::Button::Checked;
				MarkDirty(*state);
				Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
				return 0;
			}

			// Visible checkbox for selected control.
			if (id == IDC_PROP_VISIBLE && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;
				PushUndo(*state);
				auto chk = Win32::GetDlgItem(hwnd, IDC_PROP_VISIBLE);
				state->entries[sel].control->visible =
					Win32::SendMessageW(chk, Win32::Button::GetCheck, 0, 0) == Win32::Button::Checked;
				MarkDirty(*state);
				Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
				return 0;
			}

			// Enabled checkbox for selected control.
			if (id == IDC_PROP_ENABLED && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;
				PushUndo(*state);
				auto chk = Win32::GetDlgItem(hwnd, IDC_PROP_ENABLED);
				state->entries[sel].control->enabled =
					Win32::SendMessageW(chk, Win32::Button::GetCheck, 0, 0) == Win32::Button::Checked;
				MarkDirty(*state);
				Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
				return 0;
			}

			// TabStop checkbox for selected control.
			if (id == IDC_PROP_TABSTOP && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;
				PushUndo(*state);
				auto chk = Win32::GetDlgItem(hwnd, IDC_PROP_TABSTOP);
				state->entries[sel].control->tabStop =
					Win32::SendMessageW(chk, Win32::Button::GetCheck, 0, 0) == Win32::Button::Checked;
				MarkDirty(*state);
				return 0;
			}

			// GroupStart checkbox for selected control.
			if (id == IDC_PROP_GROUPSTART && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;
				PushUndo(*state);
				auto chk = Win32::GetDlgItem(hwnd, IDC_PROP_GROUPSTART);
				state->entries[sel].control->groupStart =
					Win32::SendMessageW(chk, Win32::Button::GetCheck, 0, 0) == Win32::Button::Checked;
				MarkDirty(*state);
				return 0;
			}

			// Required validation checkbox.
			if (id == IDC_PROP_VAL_REQUIRED && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;
				PushUndo(*state);
				auto chk = Win32::GetDlgItem(hwnd, IDC_PROP_VAL_REQUIRED);
				state->entries[sel].control->validation.required =
					Win32::SendMessageW(chk, Win32::Button::GetCheck, 0, 0) == Win32::Button::Checked;
				MarkDirty(*state);
				return 0;
			}

			// Window style checkboxes.
			if (code == Win32::Notifications::ButtonClicked &&
				id >= IDC_PROP_FORM_CAPTION && id <= IDC_PROP_FORM_MAXIMIZE)
			{
				PushUndo(*state);
				struct StyleBit { Win32::UINT id; Win32::DWORD flag; };
				StyleBit bits[] = {
					{ IDC_PROP_FORM_CAPTION,  Win32::Styles::Caption },
					{ IDC_PROP_FORM_SYSMENU,  Win32::Styles::SysMenu },
					{ IDC_PROP_FORM_RESIZABLE, Win32::Styles::ThickFrame },
					{ IDC_PROP_FORM_MINIMIZE, Win32::Styles::MinimizeBox },
					{ IDC_PROP_FORM_MAXIMIZE, Win32::Styles::MaximizeBox },
				};
				for (auto& b : bits)
				{
					if (b.id == id)
					{
						auto chk = Win32::GetDlgItem(hwnd, id);
						bool checked = Win32::SendMessageW(chk,
							Win32::Button::GetCheck, 0, 0) == Win32::Button::Checked;
						if (checked)
							state->form.style |= b.flag;
						else
							state->form.style &= ~b.flag;
						break;
					}
				}
				MarkDirty(*state);
				return 0;
			}

			// Control font "..." button.
			if (id == IDC_PROP_FONT_BTN && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;

				auto& ctrl = *state->entries[sel].control;
				auto resolved = FormDesigner::ResolveFont(ctrl.font, state->form.font);
				FormDesigner::FontInfo result;
				if (ShowFontDialog(*state, resolved, result))
				{
					PushUndo(*state);
					ctrl.font = result;
					RebuildSingleControl(*state, state->entries[sel]);
					UpdatePropertyPanel(*state);
					MarkDirty(*state);
				}
				return 0;
			}

			// Control font "Clear" button.
			if (id == IDC_PROP_FONT_CLEAR && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;

				PushUndo(*state);
				state->entries[sel].control->font = {};
				RebuildSingleControl(*state, state->entries[sel]);
				UpdatePropertyPanel(*state);
				MarkDirty(*state);
				return 0;
			}

			// Form font "..." button.
			if (id == IDC_PROP_FORM_FONT_BTN && code == Win32::Notifications::ButtonClicked)
			{
				auto resolved = state->form.font.isSet()
					? state->form.font
					: FormDesigner::FontInfo{ FormDesigner::DefaultFontFamily, FormDesigner::DefaultFontSize, false, false };
				FormDesigner::FontInfo result;
				if (ShowFontDialog(*state, resolved, result))
				{
					PushUndo(*state);
					state->form.font = result;
					for (auto& entry : state->entries)
						RebuildSingleControl(*state, entry);
					UpdatePropertyPanel(*state);
					MarkDirty(*state);
				}
				return 0;
			}

			// Form font "Clear" button.
			if (id == IDC_PROP_FORM_FONT_CLEAR && code == Win32::Notifications::ButtonClicked)
			{
				PushUndo(*state);
				state->form.font = {};
				for (auto& entry : state->entries)
					RebuildSingleControl(*state, entry);
				UpdatePropertyPanel(*state);
				MarkDirty(*state);
				return 0;
			}

			// Items "Edit..." button.
			if (id == IDC_PROP_ITEMS_BTN && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;

				auto& ctrl = *state->entries[sel].control;
				auto items = ctrl.items;
				if (ShowEditItemsDialog(*state, items))
				{
					PushUndo(*state);
					ctrl.items = std::move(items);
					// Clamp selectedIndex to valid range.
					if (!ctrl.items.empty())
						ctrl.selectedIndex = std::clamp(ctrl.selectedIndex, -1,
							static_cast<int>(ctrl.items.size()) - 1);
					else
						ctrl.selectedIndex = -1;
					RefreshDesignTimeItems(*state, state->entries[sel]);
					UpdatePropertyPanel(*state);
					MarkDirty(*state);
				}
				return 0;
			}

			// Image path "..." browse button.
			if (id == IDC_PROP_IMAGEPATH_BTN && code == Win32::Notifications::ButtonClicked)
			{
				int sel = SingleSelection(*state);
				if (sel < 0 || sel >= static_cast<int>(state->entries.size())) return 0;

				wchar_t fileBuf[Win32::MaxPath] = {};
				Win32::OPENFILENAMEW ofn = {};
				ofn.lStructSize = sizeof(Win32::OPENFILENAMEW);
				ofn.hwndOwner = state->surfaceHwnd;
				ofn.lpstrFilter = L"Images (*.bmp;*.ico)\0*.bmp;*.ico\0Bitmaps (*.bmp)\0*.bmp\0Icons (*.ico)\0*.ico\0All Files (*.*)\0*.*\0";
				ofn.lpstrFile = fileBuf;
				ofn.nMaxFile = Win32::MaxPath;
				ofn.Flags = Win32::FileDialog::FileMustExist | Win32::FileDialog::PathMustExist;

				if (Win32::GetOpenFileNameW(&ofn))
				{
					PushUndo(*state);
					// Make path relative to form file if possible.
					auto fullPath = std::wstring(fileBuf);
					if (!state->currentFile.empty())
					{
						auto formDir = state->currentFile.parent_path().wstring();
						if (!formDir.empty())
						{
							auto dirPrefix = formDir + L"\\";
							if (fullPath.size() > dirPrefix.size()
								&& fullPath.substr(0, dirPrefix.size()) == dirPrefix)
								fullPath = fullPath.substr(dirPrefix.size());
						}
					}
					state->entries[sel].control->imagePath = fullPath;
					// Update designer preview text.
					Win32::SetWindowTextW(state->entries[sel].hwnd,
						fullPath.empty() ? L"[Picture]" : fullPath.c_str());
					Win32::InvalidateRect(state->canvasHwnd, nullptr, true);
					UpdatePropertyPanel(*state);
					MarkDirty(*state);
				}
				return 0;
			}

			// Property edits (validate then apply on focus loss).
			if (code == Win32::Notifications::EditKillFocus)
			{
				bool isCtrlProp = (id >= IDC_PROP_TYPE && id <= IDC_PROP_TABINDEX)
					|| id == IDC_PROP_TOOLTIP || id == IDC_PROP_SELINDEX
					|| id == IDC_PROP_BINDFIELD
					|| id == IDC_PROP_ACCESSNAME || id == IDC_PROP_ACCESSDESC;
				bool isValProp = (id >= IDC_PROP_VAL_MINLEN && id <= IDC_PROP_VAL_MAX)
					|| id == IDC_PROP_VALUE;
				bool isImageProp = (id == IDC_PROP_IMAGEPATH);
				bool isFormProp = (id >= IDC_PROP_FORM_TITLE && id <= IDC_PROP_FORM_BGCOLOR)
					|| id == IDC_PROP_FORM_BINDSTRUCT;
				if (isCtrlProp || isValProp || isImageProp || isFormProp)
					ValidateAndApply(*state, hwnd, id, isFormProp);
				return 0;
			}

			// Alignment dropdown change.
			if (id == IDC_PROP_TEXTALIGN && code == Win32::Notifications::ComboBoxSelEndOk)
			{
				ApplyPropertyChange(*state, IDC_PROP_TEXTALIGN);
				return 0;
			}

			// Anchor dropdown change.
			if (id == IDC_PROP_ANCHOR && code == Win32::Notifications::ComboBoxSelEndOk)
			{
				ApplyPropertyChange(*state, IDC_PROP_ANCHOR);
				return 0;
			}
			break;
		}
		case Win32::Messages::CtlColorEdit:
		{
			if (!state) break;
			auto editHwnd = reinterpret_cast<Win32::HWND>(lParam);
			auto editId = static_cast<Win32::UINT>(Win32::GetDlgCtrlID(editHwnd));
			if (state->invalidFields.contains(editId))
			{
				auto hdc = reinterpret_cast<Win32::HDC>(wParam);
				Win32::SetTextColor(hdc, Win32::MakeRgb(128, 0, 0));
				Win32::SetBkColor(hdc, Win32::MakeRgb(255, 220, 220));
				static auto errorBrush = Win32::CreateSolidBrush(
					Win32::MakeRgb(255, 220, 220));
				return reinterpret_cast<Win32::LRESULT>(errorBrush);
			}
			break;
		}
		}

		return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
	}

}
