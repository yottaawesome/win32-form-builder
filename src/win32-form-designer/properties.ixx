export module designer:properties;
import std;
import formbuilder;
import :win32;
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
			IDC_PROP_TEXTALIGN, IDC_PROP_LOCKED, IDC_PROP_ANCHOR
		};
		constexpr Win32::UINT formIds[] = {
			IDC_PROP_FORM_TITLE, IDC_PROP_FORM_WIDTH,
			IDC_PROP_FORM_HEIGHT, IDC_PROP_FORM_BGCOLOR,
			IDC_PROP_FORM_CAPTION, IDC_PROP_FORM_SYSMENU,
			IDC_PROP_FORM_RESIZABLE, IDC_PROP_FORM_MINIMIZE, IDC_PROP_FORM_MAXIMIZE
		};

		int ctrlShow = hasSel ? Win32::Sw_Show : Win32::Sw_Hide;
		int formShow = hasSel ? Win32::Sw_Hide : Win32::Sw_Show;

		SetPropertyGroupVisibility(panel, ctrlIds, 18, ctrlShow);
		SetPropertyGroupVisibility(panel, formIds, 9, formShow);

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

		Win32::UINT editableIds[] = { IDC_PROP_TEXT, IDC_PROP_ID,
			IDC_PROP_X, IDC_PROP_Y, IDC_PROP_W, IDC_PROP_H,
			IDC_PROP_ONCLICK, IDC_PROP_ONCHANGE, IDC_PROP_ONDBLCLICK, IDC_PROP_ONSELCHANGE,
			IDC_PROP_ONFOCUS, IDC_PROP_ONBLUR, IDC_PROP_ONCHECK, IDC_PROP_TABINDEX };
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
				case IDC_PROP_W: if (val >= MIN_CONTROL_SIZE) ctrl.rect.width = val; break;
				case IDC_PROP_H: if (val >= MIN_CONTROL_SIZE) ctrl.rect.height = val; break;
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
			if (val < MIN_CONTROL_SIZE)
				return std::format(L"Must be >= {}", MIN_CONTROL_SIZE);
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

		auto header = Win32::CreateWindowExW(0, L"STATIC", L"Properties",
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticLeft,
			5, 5, 210, 20, parent, nullptr, hInst, nullptr);
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
		};

		int y = 30;
		for (auto& row : ctrlRows)
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", row.label,
				Win32::Styles::Child | Win32::Styles::StaticRight,
				5, y + 2, 55, 18, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::TabStop | row.extraStyle,
				65, y, 150, 22, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);
			Win32::EnableWindow(edit, false);

			y += 26;
		}

		// Text alignment dropdown (ComboBox instead of EDIT).
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Align:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				5, y + 2, 55, 18, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_TEXTALIGN + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto combo = Win32::CreateWindowExW(0, Win32::Controls::ComboBox, nullptr,
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::ComboBoxDropDownList,
				65, y, 150, 120, parent,
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
		y += 26;
		{
			auto chk = Win32::CreateWindowExW(0, Win32::Controls::Button, L"Locked",
				Win32::Styles::Child | Win32::Styles::AutoCheckBox,
				15, y, 200, 20, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_LOCKED)),
				hInst, nullptr);
			Win32::SendMessageW(chk, Win32::Messages::SetFont, font, true);
		}

		// Anchor dropdown.
		y += 24;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Anchor:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				5, y + 2, 55, 18, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_ANCHOR + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto combo = Win32::CreateWindowExW(0, Win32::Controls::ComboBox, nullptr,
				Win32::Styles::Child | Win32::Styles::TabStop | Win32::Styles::ComboBoxDropDownList,
				65, y, 150, 200, parent,
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
		y += 26;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Font:",
				Win32::Styles::Child | Win32::Styles::StaticRight,
				5, y + 2, 55, 18, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FONT_LABEL + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto fontLabel = Win32::CreateWindowExW(0, L"STATIC", L"(inherited)",
				Win32::Styles::Child | Win32::Styles::StaticLeft,
				65, y + 2, 85, 18, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FONT_LABEL)),
				hInst, nullptr);
			Win32::SendMessageW(fontLabel, Win32::Messages::SetFont, font, true);

			auto fontBtn = Win32::CreateWindowExW(0, L"BUTTON", L"...",
				Win32::Styles::Child | Win32::Styles::ButtonPush,
				155, y, 30, 22, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FONT_BTN)),
				hInst, nullptr);
			Win32::SendMessageW(fontBtn, Win32::Messages::SetFont, font, true);

			auto fontClear = Win32::CreateWindowExW(0, L"BUTTON", L"X",
				Win32::Styles::Child | Win32::Styles::ButtonPush,
				190, y, 25, 22, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FONT_CLEAR)),
				hInst, nullptr);
			Win32::SendMessageW(fontClear, Win32::Messages::SetFont, font, true);
		}

		// Form property rows (visible when no control is selected).
		PropRow formRows[] = {
			{ L"Title:",   IDC_PROP_FORM_TITLE,  Win32::Styles::EditAutoHScroll },
			{ L"Width:",   IDC_PROP_FORM_WIDTH,  Win32::Styles::EditAutoHScroll },
			{ L"Height:",  IDC_PROP_FORM_HEIGHT, Win32::Styles::EditAutoHScroll },
			{ L"BgColor:", IDC_PROP_FORM_BGCOLOR, Win32::Styles::EditAutoHScroll },
		};

		y = 30;
		for (auto& row : formRows)
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", row.label,
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticRight,
				5, y + 2, 55, 18, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			int editW = (row.editId == IDC_PROP_FORM_BGCOLOR) ? 105 : 150;
			auto edit = Win32::CreateWindowExW(Win32::ExStyles::ClientEdge, L"EDIT", L"",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::TabStop | row.extraStyle,
				65, y, editW, 22, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(row.editId)),
				hInst, nullptr);
			Win32::SendMessageW(edit, Win32::Messages::SetFont, font, true);

			y += 26;
		}

		// Color picker button next to BgColor.
		auto bgBtn = Win32::CreateWindowExW(0, L"BUTTON", L"...",
			Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
			175, 30 + 3 * 26, 40, 22, parent,
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

		y = 30 + 4 * 26 + 10; // After the 4 form property edit rows + spacing.
		auto styleHeader = Win32::CreateWindowExW(0, L"STATIC", L"Window Style",
			Win32::Styles::Child | Win32::Styles::Visible,
			5, y, 210, 18, parent,
			reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_CAPTION + IDL_OFFSET)),
			hInst, nullptr);
		Win32::SendMessageW(styleHeader, Win32::Messages::SetFont, font, true);
		y += 22;

		for (auto& sc : styleChecks)
		{
			auto chk = Win32::CreateWindowExW(0, Win32::Controls::Button, sc.label,
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::AutoCheckBox,
				15, y, 200, 20, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(sc.id)),
				hInst, nullptr);
			Win32::SendMessageW(chk, Win32::Messages::SetFont, font, true);
			y += 22;
		}

		// Form font row: label + "..." button + "Clear" button.
		y += 8;
		{
			auto lbl = Win32::CreateWindowExW(0, L"STATIC", L"Font:",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticRight,
				5, y + 2, 55, 18, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_FONT_LABEL + IDL_OFFSET)),
				hInst, nullptr);
			Win32::SendMessageW(lbl, Win32::Messages::SetFont, font, true);

			auto fontLabel = Win32::CreateWindowExW(0, L"STATIC", L"(default)",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::StaticLeft,
				65, y + 2, 85, 18, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_FONT_LABEL)),
				hInst, nullptr);
			Win32::SendMessageW(fontLabel, Win32::Messages::SetFont, font, true);

			auto fontBtn = Win32::CreateWindowExW(0, L"BUTTON", L"...",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
				155, y, 30, 22, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_FONT_BTN)),
				hInst, nullptr);
			Win32::SendMessageW(fontBtn, Win32::Messages::SetFont, font, true);

			auto fontClear = Win32::CreateWindowExW(0, L"BUTTON", L"X",
				Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::ButtonPush,
				190, y, 25, 22, parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::UINT_PTR>(IDC_PROP_FORM_FONT_CLEAR)),
				hInst, nullptr);
			Win32::SendMessageW(fontClear, Win32::Messages::SetFont, font, true);
		}
	}

	constexpr int PROP_CONTENT_CTRL = 30 + 18 * 26 + 10;  // control properties + font row: 508px
	constexpr int PROP_CONTENT_FORM = 30 + 4 * 26 + 10 + 22 + 5 * 22 + 8 + 26 + 10;  // form properties + style checkboxes + font row: 298px
	constexpr int SCROLL_LINE = 26;                         // one row height

	void UpdateScrollRange(DesignState& state)
	{
		auto panel = state.propertyHwnd;
		int sel = SingleSelection(state);
		bool hasSel = sel >= 0 && sel < static_cast<int>(state.entries.size());
		int contentHeight = hasSel ? PROP_CONTENT_CTRL : PROP_CONTENT_FORM;

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

			switch (Win32::GetLowWord(wParam))
			{
			case Win32::ScrollBar::LineUp:        pos -= SCROLL_LINE; break;
			case Win32::ScrollBar::LineDown:      pos += SCROLL_LINE; break;
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
			ScrollPropertyPanel(*state, state->propertyScrollY - lines * SCROLL_LINE);
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

			// Property edits (validate then apply on focus loss).
			if (code == Win32::Notifications::EditKillFocus)
			{
				bool isCtrlProp = (id >= IDC_PROP_TYPE && id <= IDC_PROP_TABINDEX);
				bool isFormProp = (id >= IDC_PROP_FORM_TITLE && id <= IDC_PROP_FORM_BGCOLOR);
				if (isCtrlProp || isFormProp)
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
