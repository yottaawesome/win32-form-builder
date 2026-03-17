export module formbuilder:codegen;
import std;
import :win32;
import :schema;

namespace FormDesigner
{
	// Converts a snake_case event name to a PascalCase function name prefixed with "On".
	auto ToPascalCase(const std::string& name) -> std::string
	{
		auto result = std::string{"On"};
		auto capitalize = true;
		for (auto ch : name)
		{
			if (ch == '_') { capitalize = true; continue; }
			result += capitalize ? static_cast<char>(std::toupper(static_cast<unsigned char>(ch))) : ch;
			capitalize = false;
		}
		return result;
	}

	// Returns the Win32 class name macro for a ControlType (e.g. "WC_BUTTON").
	auto Win32ClassLiteral(ControlType type) -> const char*
	{
		switch (type)
		{
		case ControlType::Button:
		case ControlType::CheckBox:
		case ControlType::RadioButton:
		case ControlType::GroupBox:          return "WC_BUTTON";
		case ControlType::Label:             return "WC_STATIC";
		case ControlType::TextBox:           return "WC_EDIT";
		case ControlType::ListBox:           return "WC_LISTBOX";
		case ControlType::ComboBox:          return "WC_COMBOBOX";
		case ControlType::ProgressBar:       return "PROGRESS_CLASS";
		case ControlType::TrackBar:          return "TRACKBAR_CLASS";
		case ControlType::DateTimePicker:    return "DATETIMEPICK_CLASS";
		case ControlType::TabControl:        return "WC_TABCONTROL";
		case ControlType::ListView:          return "WC_LISTVIEW";
		case ControlType::TreeView:          return "WC_TREEVIEW";
		case ControlType::UpDown:            return "UPDOWN_CLASS";
		case ControlType::RichEdit:          return "MSFTEDIT_CLASS";
		case ControlType::MonthCalendar:     return "MONTHCAL_CLASS";
		case ControlType::Link:              return "WC_LINK";
		case ControlType::IPAddress:         return "WC_IPADDRESS";
		case ControlType::HotKey:            return "HOTKEY_CLASS";
		case ControlType::Picture:           return "WC_STATIC";
		case ControlType::Separator:         return "WC_STATIC";
		case ControlType::Animation:         return "ANIMATE_CLASS";
		default:                             return "L\"Window\"";
		}
	}

	// Returns a human-readable type prefix for variable and ID names.
	auto TypePrefix(ControlType type) -> const char*
	{
		switch (type)
		{
		case ControlType::Button:            return "Button";
		case ControlType::CheckBox:          return "CheckBox";
		case ControlType::RadioButton:       return "RadioButton";
		case ControlType::Label:             return "Label";
		case ControlType::TextBox:           return "TextBox";
		case ControlType::GroupBox:          return "GroupBox";
		case ControlType::ListBox:           return "ListBox";
		case ControlType::ComboBox:          return "ComboBox";
		case ControlType::ProgressBar:       return "ProgressBar";
		case ControlType::TrackBar:          return "TrackBar";
		case ControlType::DateTimePicker:    return "DateTimePicker";
		case ControlType::TabControl:        return "TabControl";
		case ControlType::ListView:          return "ListView";
		case ControlType::TreeView:          return "TreeView";
		case ControlType::UpDown:            return "UpDown";
		case ControlType::RichEdit:          return "RichEdit";
		case ControlType::MonthCalendar:     return "MonthCal";
		case ControlType::Link:              return "Link";
		case ControlType::IPAddress:         return "IPAddress";
		case ControlType::HotKey:            return "HotKey";
		case ControlType::Picture:           return "Picture";
		case ControlType::Separator:         return "Separator";
		case ControlType::Animation:         return "Animation";
		default:                             return "Control";
		}
	}

	// Returns an upper-case type prefix for #define IDC_ constants.
	auto TypePrefixUpper(ControlType type) -> const char*
	{
		switch (type)
		{
		case ControlType::Button:            return "BUTTON";
		case ControlType::CheckBox:          return "CHECKBOX";
		case ControlType::RadioButton:       return "RADIOBUTTON";
		case ControlType::Label:             return "LABEL";
		case ControlType::TextBox:           return "TEXTBOX";
		case ControlType::GroupBox:          return "GROUPBOX";
		case ControlType::ListBox:           return "LISTBOX";
		case ControlType::ComboBox:          return "COMBOBOX";
		case ControlType::ProgressBar:       return "PROGRESSBAR";
		case ControlType::TrackBar:          return "TRACKBAR";
		case ControlType::DateTimePicker:    return "DATETIMEPICKER";
		case ControlType::TabControl:        return "TABCONTROL";
		case ControlType::ListView:          return "LISTVIEW";
		case ControlType::TreeView:          return "TREEVIEW";
		case ControlType::UpDown:            return "UPDOWN";
		case ControlType::RichEdit:          return "RICHEDIT";
		case ControlType::MonthCalendar:     return "MONTHCAL";
		case ControlType::Link:              return "LINK";
		case ControlType::IPAddress:         return "IPADDRESS";
		case ControlType::HotKey:            return "HOTKEY";
		case ControlType::Picture:           return "PICTURE";
		case ControlType::Separator:         return "SEPARATOR";
		case ControlType::Animation:         return "ANIMATION";
		default:                             return "CONTROL";
		}
	}

	// Data binding type categories for code generation.
	enum class BindType { Text, Bool, Int, Index, None };

	auto BindTypeFor(ControlType type) -> BindType
	{
		switch (type)
		{
		case ControlType::TextBox:
		case ControlType::RichEdit:
		case ControlType::Label:
		case ControlType::Button:
		case ControlType::IPAddress:
			return BindType::Text;
		case ControlType::CheckBox:
		case ControlType::RadioButton:
			return BindType::Bool;
		case ControlType::TrackBar:
		case ControlType::UpDown:
		case ControlType::ProgressBar:
			return BindType::Int;
		case ControlType::ComboBox:
		case ControlType::ListBox:
			return BindType::Index;
		default:
			return BindType::None;
		}
	}

	auto BindTypeName(BindType bt) -> const char*
	{
		switch (bt)
		{
		case BindType::Text:  return "std::wstring";
		case BindType::Bool:  return "bool";
		case BindType::Int:   return "int";
		case BindType::Index: return "int";
		default:              return "/* unsupported */";
		}
	}

	// Builds a human-readable style expression string (e.g. "WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX").
	auto BuildStyleExpression(ControlType type, TextAlign align, Win32::DWORD customStyle, bool visible = true, bool enabled = true, bool tabStop = true, bool groupStart = false) -> std::string
	{
		auto parts = std::vector<std::string>{"WS_CHILD"};
		if (visible)
			parts.push_back("WS_VISIBLE");
		if (!enabled)
			parts.push_back("WS_DISABLED");
		if (tabStop && IsInteractiveControl(type))
			parts.push_back("WS_TABSTOP");
		if (groupStart)
			parts.push_back("WS_GROUP");

		switch (type)
		{
		case ControlType::CheckBox:    parts.push_back("BS_AUTOCHECKBOX"); break;
		case ControlType::RadioButton: parts.push_back("BS_AUTORADIOBUTTON"); break;
		case ControlType::GroupBox:    parts.push_back("BS_GROUPBOX"); break;
		case ControlType::TextBox:     parts.push_back("WS_BORDER"); parts.push_back("ES_AUTOHSCROLL"); break;
		case ControlType::ListBox:     parts.push_back("LBS_STANDARD"); break;
		case ControlType::ComboBox:    parts.push_back("CBS_DROPDOWNLIST"); break;
		case ControlType::ListView:    parts.push_back("LVS_REPORT"); parts.push_back("LVS_SHOWSELALWAYS"); break;
		case ControlType::TreeView:    parts.push_back("TVS_HASBUTTONS"); parts.push_back("TVS_HASLINES"); parts.push_back("TVS_LINESATROOT"); break;
		case ControlType::RichEdit:    parts.push_back("WS_BORDER"); parts.push_back("ES_MULTILINE"); parts.push_back("ES_AUTOVSCROLL"); break;
		case ControlType::Picture:     parts.push_back("SS_ETCHEDFRAME"); break;
		case ControlType::Separator:   parts.push_back("SS_ETCHEDHORZ"); break;
		default: break;
		}

		switch (type)
		{
		case ControlType::Label:
			switch (align)
			{
			case TextAlign::Center: parts.push_back("SS_CENTER"); break;
			case TextAlign::Right:  parts.push_back("SS_RIGHT"); break;
			default: break;
			}
			break;
		case ControlType::TextBox:
		case ControlType::RichEdit:
			switch (align)
			{
			case TextAlign::Center: parts.push_back("ES_CENTER"); break;
			case TextAlign::Right:  parts.push_back("ES_RIGHT"); break;
			default: break;
			}
			break;
		case ControlType::Button:
		case ControlType::CheckBox:
		case ControlType::RadioButton:
			switch (align)
			{
			case TextAlign::Left:   parts.push_back("BS_LEFT"); break;
			case TextAlign::Center: parts.push_back("BS_CENTER"); break;
			case TextAlign::Right:  parts.push_back("BS_RIGHT"); break;
			}
			break;
		default: break;
		}

		if (customStyle != 0)
			parts.push_back(std::format("0x{:X}", customStyle));

		auto result = std::string{};
		for (size_t i = 0; i < parts.size(); ++i)
		{
			if (i > 0) result += " | ";
			result += parts[i];
		}
		return result;
	}

	// Builds a window style expression for the form (e.g. "WS_OVERLAPPEDWINDOW").
	auto BuildFormStyleExpression(Win32::DWORD style, bool enabled = true) -> std::string
	{
		if (style == Win32::Styles::OverlappedWindow && enabled)
			return "WS_OVERLAPPEDWINDOW";

		auto parts = std::vector<std::string>{};
		if (style & Win32::Styles::Caption)     parts.push_back("WS_CAPTION");
		if (style & Win32::Styles::SysMenu)     parts.push_back("WS_SYSMENU");
		if (style & Win32::Styles::ThickFrame)  parts.push_back("WS_THICKFRAME");
		if (style & Win32::Styles::MinimizeBox) parts.push_back("WS_MINIMIZEBOX");
		if (style & Win32::Styles::MaximizeBox) parts.push_back("WS_MAXIMIZEBOX");
		if (!enabled)
			parts.push_back("WS_DISABLED");

		if (parts.empty())
			return std::format("0x{:X}UL", style);

		auto result = std::string{};
		for (size_t i = 0; i < parts.size(); ++i)
		{
			if (i > 0) result += " | ";
			result += parts[i];
		}
		return result;
	}

	// Escapes a wstring for use in a C++ L"..." literal.
	auto EscapeWString(const std::wstring& s) -> std::string
	{
		auto result = std::string{};
		for (auto ch : s)
		{
			switch (ch)
			{
			case L'\\': result += "\\\\"; break;
			case L'"':  result += "\\\""; break;
			case L'\n': result += "\\n"; break;
			case L'\r': result += "\\r"; break;
			case L'\t': result += "\\t"; break;
			default:
				if (ch >= 32 && ch < 127)
					result += static_cast<char>(ch);
				else
					result += std::format("\\x{:04X}", static_cast<unsigned int>(ch));
				break;
			}
		}
		return result;
	}

	// Returns the IDC_ constant name for a control.
	auto IdcConstantName(const Control& ctrl) -> std::string
	{
		return std::format("IDC_{}_{}", TypePrefixUpper(ctrl.type), ctrl.id);
	}

	// Returns a short PascalCase type suffix for control ID constant names.
	auto TypeIdSuffix(ControlType type) -> const char*
	{
		switch (type)
		{
		case ControlType::Button:            return "Button";
		case ControlType::CheckBox:          return "Check";
		case ControlType::RadioButton:       return "Radio";
		case ControlType::Label:             return "Label";
		case ControlType::TextBox:           return "Text";
		case ControlType::GroupBox:          return "Group";
		case ControlType::ListBox:           return "List";
		case ControlType::ComboBox:          return "Combo";
		case ControlType::ProgressBar:       return "Progress";
		case ControlType::TrackBar:          return "Track";
		case ControlType::DateTimePicker:    return "DateTime";
		case ControlType::TabControl:        return "Tab";
		case ControlType::ListView:          return "ListView";
		case ControlType::TreeView:          return "TreeView";
		case ControlType::UpDown:            return "UpDown";
		case ControlType::RichEdit:          return "RichEdit";
		case ControlType::MonthCalendar:     return "Calendar";
		case ControlType::Link:              return "Link";
		case ControlType::IPAddress:         return "IpAddress";
		case ControlType::HotKey:            return "HotKey";
		case ControlType::Picture:           return "Picture";
		case ControlType::Separator:         return "Separator";
		case ControlType::Animation:         return "Animation";
		default:                             return "Control";
		}
	}

	// Sanitizes control text into a PascalCase C++ identifier fragment.
	// Strips accelerator prefixes (&), trailing colons, and non-alphanumeric characters.
	auto SanitizeIdentifier(const std::wstring& text) -> std::string
	{
		auto result = std::string{};
		auto capitalize = true;
		for (auto ch : text)
		{
			if (ch == L'&') continue;
			if (ch == L' ' || ch == L'-' || ch == L'_' || ch == L'\t')
			{
				capitalize = true;
				continue;
			}
			if (ch == L':' || ch == L'.' || ch == L',' || ch == L';' ||
				ch == L'!' || ch == L'?' || ch == L'(' || ch == L')' ||
				ch == L'[' || ch == L']' || ch == L'{' || ch == L'}' ||
				ch == L'/' || ch == L'\\' || ch == L'<' || ch == L'>' ||
				ch == L'=' || ch == L'+' || ch == L'*' || ch == L'#' ||
				ch == L'@' || ch == L'%' || ch == L'^' || ch == L'~' ||
				ch == L'`' || ch == L'|' || ch == L'"' || ch == L'\'')
				continue;
			if (ch < 128 && std::isalnum(static_cast<unsigned char>(ch)))
			{
				if (capitalize)
					result += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
				else
					result += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
				capitalize = false;
			}
		}
		return result;
	}

	// Builds a PascalCase constant name for a control (e.g. "OkButton", "UserNameText").
	auto BuildControlIdName(const Control& ctrl) -> std::string
	{
		auto base = SanitizeIdentifier(ctrl.text);
		auto suffix = std::string{TypeIdSuffix(ctrl.type)};
		if (base.empty())
			return suffix + std::to_string(ctrl.id);
		return base + suffix;
	}

	// Returns a local variable name for a control (e.g. "hButton301").
	auto ControlVarName(const Control& ctrl, int index) -> std::string
	{
		if (ctrl.id != 0)
			return std::format("h{}{}", TypePrefix(ctrl.type), ctrl.id);
		return std::format("h{}{}", TypePrefix(ctrl.type), index + 1);
	}

	// Checks whether any control in the tree uses RichEdit.
	auto NeedsRichEdit(std::span<const Control> controls) -> bool
	{
		for (auto& c : controls)
		{
			if (c.type == ControlType::RichEdit) return true;
			if (NeedsRichEdit(c.children)) return true;
		}
		return false;
	}

	// Checks whether any control in the tree has a tooltip.
	auto NeedsTooltips(std::span<const Control> controls) -> bool
	{
		for (auto& c : controls)
		{
			if (!c.tooltip.empty()) return true;
			if (NeedsTooltips(c.children)) return true;
		}
		return false;
	}

	// Event dispatch info collected from controls.
	struct DispatchEntry
	{
		int controlId;
		std::string idcName;
		std::string functionName;
	};

	struct EventDispatch
	{
		// WM_COMMAND groups keyed by notification constant name.
		std::map<std::string, std::vector<DispatchEntry>> command;
		// WM_NOTIFY groups keyed by notification constant name.
		std::map<std::string, std::vector<DispatchEntry>> notify;
		// Controls needing onCheck (BN_CLICKED then BM_GETCHECK).
		std::vector<DispatchEntry> checkEntries;
		// Unique event handler stubs: functionName → eventType.
		std::map<std::string, std::string> stubs;
	};

	void CollectEvents(const Control& ctrl, EventDispatch& dispatch)
	{
		auto idc = ctrl.id != 0 ? IdcConstantName(ctrl) : std::format("{}", ctrl.id);

		auto addCommand = [&](const std::string& handler, const std::string& notifCode, const std::string& eventType)
		{
			if (handler.empty() || ctrl.id == 0) return;
			auto funcName = ToPascalCase(handler);
			dispatch.command[notifCode].push_back({ ctrl.id, idc, funcName });
			dispatch.stubs.try_emplace(funcName, eventType);
		};

		auto addNotify = [&](const std::string& handler, const std::string& notifCode, const std::string& eventType)
		{
			if (handler.empty() || ctrl.id == 0) return;
			auto funcName = ToPascalCase(handler);
			dispatch.notify[notifCode].push_back({ ctrl.id, idc, funcName });
			dispatch.stubs.try_emplace(funcName, eventType);
		};

		// onClick → BN_CLICKED for buttons, NM_CLICK for Link (SysLink)
		if (!ctrl.onClick.empty())
		{
			if (ctrl.type == ControlType::Link)
				addNotify(ctrl.onClick, "NM_CLICK", "click");
			else
				addCommand(ctrl.onClick, "BN_CLICKED", "click");
		}

		// onCheck → BN_CLICKED then BM_GETCHECK (CheckBox, RadioButton)
		if (!ctrl.onCheck.empty() && ctrl.id != 0)
		{
			auto funcName = ToPascalCase(ctrl.onCheck);
			dispatch.checkEntries.push_back({ ctrl.id, idc, funcName });
			dispatch.stubs.try_emplace(funcName, "check");
		}

		// onChange → depends on control type
		if (!ctrl.onChange.empty())
		{
			switch (ctrl.type)
			{
			case ControlType::TextBox:
			case ControlType::RichEdit:
				addCommand(ctrl.onChange, "EN_CHANGE", "change");
				break;
			case ControlType::ComboBox:
				addCommand(ctrl.onChange, "CBN_SELCHANGE", "change");
				break;
			case ControlType::ListBox:
				addCommand(ctrl.onChange, "LBN_SELCHANGE", "change");
				break;
			case ControlType::DateTimePicker:
				addNotify(ctrl.onChange, "DTN_DATETIMECHANGE", "change");
				break;
			case ControlType::MonthCalendar:
				addNotify(ctrl.onChange, "MCN_SELCHANGE", "change");
				break;
			case ControlType::IPAddress:
				addNotify(ctrl.onChange, "IPN_FIELDCHANGED", "change");
				break;
			default:
				addCommand(ctrl.onChange, "EN_CHANGE", "change");
				break;
			}
		}

		// onDoubleClick → depends on control type
		if (!ctrl.onDoubleClick.empty())
		{
			switch (ctrl.type)
			{
			case ControlType::ListView:
			case ControlType::TreeView:
				addNotify(ctrl.onDoubleClick, "NM_DBLCLK", "doubleclick");
				break;
			case ControlType::ListBox:
				addCommand(ctrl.onDoubleClick, "LBN_DBLCLK", "doubleclick");
				break;
			default:
				addCommand(ctrl.onDoubleClick, "BN_DBLCLK", "doubleclick");
				break;
			}
		}

		// onFocus → depends on control type
		if (!ctrl.onFocus.empty())
		{
			switch (ctrl.type)
			{
			case ControlType::ListBox:
				addCommand(ctrl.onFocus, "LBN_SETFOCUS", "focus");
				break;
			case ControlType::ComboBox:
				addCommand(ctrl.onFocus, "CBN_SETFOCUS", "focus");
				break;
			default:
				addCommand(ctrl.onFocus, "EN_SETFOCUS", "focus");
				break;
			}
		}

		// onBlur → depends on control type
		if (!ctrl.onBlur.empty())
		{
			switch (ctrl.type)
			{
			case ControlType::ListBox:
				addCommand(ctrl.onBlur, "LBN_KILLFOCUS", "blur");
				break;
			case ControlType::ComboBox:
				addCommand(ctrl.onBlur, "CBN_KILLFOCUS", "blur");
				break;
			default:
				addCommand(ctrl.onBlur, "EN_KILLFOCUS", "blur");
				break;
			}
		}

		// onSelectionChange → WM_NOTIFY
		if (!ctrl.onSelectionChange.empty())
		{
			switch (ctrl.type)
			{
			case ControlType::TreeView:
				addNotify(ctrl.onSelectionChange, "TVN_SELCHANGEDW", "selectionchange");
				break;
			case ControlType::ListView:
				addNotify(ctrl.onSelectionChange, "LVN_ITEMCHANGED", "selectionchange");
				break;
			case ControlType::TabControl:
				addNotify(ctrl.onSelectionChange, "TCN_SELCHANGE", "selectionchange");
				break;
			default: break;
			}
		}

		for (auto& child : ctrl.children)
			CollectEvents(child, dispatch);
	}

	// Generates a window class name from the form title.
	auto MakeClassName(const std::wstring& title) -> std::string
	{
		auto result = std::string{};
		auto capitalize = true;
		for (auto ch : title)
		{
			if (ch == L' ' || ch == L'-' || ch == L'_') { capitalize = true; continue; }
			if ((ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z') || (ch >= L'0' && ch <= L'9'))
			{
				result += capitalize ? static_cast<char>(std::toupper(static_cast<unsigned char>(ch))) : static_cast<char>(ch);
				capitalize = false;
			}
		}
		if (result.empty()) result = "MainWindow";
		return result + "Class";
	}

	// Key for identifying unique font configurations in codegen.
	struct FontKey
	{
		std::wstring family;
		int size;
		bool bold;
		bool italic;

		auto operator<=>(const FontKey&) const = default;
		bool operator==(const FontKey&) const = default;
		auto operator<(const FontKey& other) const -> bool
		{
			if (family != other.family) return family < other.family;
			if (size != other.size) return size < other.size;
			if (bold != other.bold) return bold < other.bold;
			return italic < other.italic;
		}
	};

	// Collects unique fonts used across the form. Returns map of FontKey → variable name.
	void CollectFonts(std::span<const Control> controls, const FontInfo& formFont,
		std::map<FontKey, std::string>& fontMap, int& fontIndex)
	{
		for (auto& ctrl : controls)
		{
			auto resolved = ResolveFont(ctrl.font, formFont);
			auto key = FontKey{ resolved.family, resolved.size, resolved.bold, resolved.italic };
			if (!fontMap.contains(key))
				fontMap[key] = std::format("hFont{}", fontIndex++);

			if (!ctrl.children.empty())
				CollectFonts(ctrl.children, formFont, fontMap, fontIndex);
		}
	}

	// Returns the font variable name for a given control's resolved font.
	auto FontVarForControl(const Control& ctrl, const FontInfo& formFont,
		const std::map<FontKey, std::string>& fontMap) -> std::string
	{
		auto resolved = ResolveFont(ctrl.font, formFont);
		auto key = FontKey{ resolved.family, resolved.size, resolved.bold, resolved.italic };
		auto it = fontMap.find(key);
		return it != fontMap.end() ? it->second : "hFont";
	}

	// Emits CreateWindowExW calls for a list of controls.
	void EmitControlCreation(std::ostringstream& out, std::span<const Control> controls,
		const std::string& parentVar, int& controlIndex, const std::string& indent,
		const FontInfo& formFont, const std::map<FontKey, std::string>& fontMap,
		bool hasTooltips)
	{
		for (auto& ctrl : controls)
		{
			auto varName = ControlVarName(ctrl, controlIndex);
			auto className = Win32ClassLiteral(ctrl.type);
			auto styleExpr = BuildStyleExpression(ctrl.type, ctrl.textAlign, ctrl.style, ctrl.visible, ctrl.enabled, ctrl.tabStop, ctrl.groupStart);
			auto textLiteral = std::format("L\"{}\"", EscapeWString(ctrl.text));
			auto menuExpr = ctrl.id != 0
				? std::format("(HMENU){}", IdcConstantName(ctrl))
				: std::string{"NULL"};

			out << indent << "// " << TypePrefix(ctrl.type);
			if (!ctrl.text.empty())
			{
				out << ": \"" << ToNarrow(ctrl.text) << "\"";
			}
			if (ctrl.id != 0)
				out << " (ID: " << ctrl.id << ")";
			out << "\n";

			// Emit validation metadata as comments.
			if (ctrl.validation.isSet())
			{
				out << indent << "// Validation:";
				if (ctrl.validation.required)
					out << " required";
				if (ctrl.validation.minLength != 0)
					out << " minLength=" << ctrl.validation.minLength;
				if (ctrl.validation.maxLength != 0)
					out << " maxLength=" << ctrl.validation.maxLength;
				if (!ctrl.validation.pattern.empty())
					out << " pattern=\"" << ctrl.validation.pattern << "\"";
				if (ctrl.validation.min != 0)
					out << " min=" << ctrl.validation.min;
				if (ctrl.validation.max != 0)
					out << " max=" << ctrl.validation.max;
				out << "\n";
			}

			// Emit accessibility metadata as comments.
			if (!ctrl.accessibleName.empty())
				out << indent << "// Accessible name: \"" << ToNarrow(ctrl.accessibleName) << "\"\n";
			if (!ctrl.accessibleDescription.empty())
				out << indent << "// Accessible description: \"" << ToNarrow(ctrl.accessibleDescription) << "\"\n";

			out << indent << "HWND " << varName << " = CreateWindowExW(\n";
			out << indent << "    " << (ctrl.exStyle != 0 ? std::format("0x{:X}", ctrl.exStyle) : "0") << ",\n";
			out << indent << "    " << className << ", " << textLiteral << ",\n";
			out << indent << "    " << styleExpr << ",\n";
			out << indent << "    MulDiv(" << ctrl.rect.x << ", dpi, 96), MulDiv(" << ctrl.rect.y << ", dpi, 96), "
				<< "MulDiv(" << ctrl.rect.width << ", dpi, 96), MulDiv(" << ctrl.rect.height << ", dpi, 96),\n";
			out << indent << "    " << parentVar << ", " << menuExpr << ", hInstance, NULL);\n";

			auto fontVar = FontVarForControl(ctrl, formFont, fontMap);
			out << indent << "SendMessageW(" << varName << ", WM_SETFONT, (WPARAM)" << fontVar << ", TRUE);\n";

			if (hasTooltips && !ctrl.tooltip.empty())
			{
				auto tipLiteral = std::format("L\"{}\"", EscapeWString(ctrl.tooltip));
				out << indent << "{ TOOLINFO ti = { sizeof(TOOLINFO), TTF_SUBCLASS | TTF_IDISHWND, "
					<< parentVar << ", (UINT_PTR)" << varName << ", {}, " << tipLiteral << " };\n";
				out << indent << "  SendMessageW(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti); }\n";
			}

			// Emit ComboBox/ListBox item population.
			if (!ctrl.items.empty() &&
				(ctrl.type == ControlType::ComboBox || ctrl.type == ControlType::ListBox))
			{
				auto addMsgName = (ctrl.type == ControlType::ComboBox) ? "CB_ADDSTRING" : "LB_ADDSTRING";
				for (auto& item : ctrl.items)
				{
					auto itemLiteral = std::format("L\"{}\"", EscapeWString(item));
					out << indent << "SendMessageW(" << varName << ", " << addMsgName
						<< ", 0, (LPARAM)" << itemLiteral << ");\n";
				}
				if (ctrl.selectedIndex >= 0)
				{
					auto selMsgName = (ctrl.type == ControlType::ComboBox) ? "CB_SETCURSEL" : "LB_SETCURSEL";
					out << indent << "SendMessageW(" << varName << ", " << selMsgName
						<< ", " << ctrl.selectedIndex << ", 0);\n";
				}
			}

			// Emit image loading for Picture controls.
			if (ctrl.type == ControlType::Picture && !ctrl.imagePath.empty())
			{
				auto imgType = ImageTypeFromPath(ctrl.imagePath);
				if (imgType > 0)
				{
					auto pathLiteral = std::format("L\"{}\"", ToNarrow(ctrl.imagePath));
					auto imageTypeName = (imgType == 1) ? "IMAGE_BITMAP" : "IMAGE_ICON";
					auto styleName = (imgType == 1) ? "SS_BITMAP" : "SS_ICON";
					out << indent << "{\n";
					out << indent << "    HANDLE hImg = LoadImageW(NULL, " << pathLiteral
						<< ", " << imageTypeName << ", 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);\n";
					out << indent << "    if (hImg) {\n";
					out << indent << "        LONG_PTR style = GetWindowLongPtrW(" << varName << ", GWL_STYLE);\n";
					out << indent << "        style = (style & ~SS_ETCHEDFRAME) | " << styleName << " | SS_CENTERIMAGE;\n";
					out << indent << "        SetWindowLongPtrW(" << varName << ", GWL_STYLE, style);\n";
					out << indent << "        SendMessageW(" << varName << ", STM_SETIMAGE, "
						<< imageTypeName << ", (LPARAM)hImg);\n";
					out << indent << "    }\n";
					out << indent << "}\n";
				}
			}

			// Emit range and value initialization for numeric controls.
			if (SupportsValue(ctrl.type))
			{
				if (ctrl.validation.min != 0 || ctrl.validation.max != 0)
				{
					switch (ctrl.type)
					{
					case ControlType::ProgressBar:
						out << indent << "SendMessageW(" << varName << ", PBM_SETRANGE32, "
							<< ctrl.validation.min << ", " << ctrl.validation.max << ");\n";
						break;
					case ControlType::TrackBar:
						out << indent << "SendMessageW(" << varName << ", TBM_SETRANGEMIN, FALSE, "
							<< ctrl.validation.min << ");\n";
						out << indent << "SendMessageW(" << varName << ", TBM_SETRANGEMAX, TRUE, "
							<< ctrl.validation.max << ");\n";
						break;
					case ControlType::UpDown:
						out << indent << "SendMessageW(" << varName << ", UDM_SETRANGE32, "
							<< ctrl.validation.min << ", " << ctrl.validation.max << ");\n";
						break;
					default: break;
					}
				}
				if (ctrl.value != 0)
				{
					switch (ctrl.type)
					{
					case ControlType::ProgressBar:
						out << indent << "SendMessageW(" << varName << ", PBM_SETPOS, "
							<< ctrl.value << ", 0);\n";
						break;
					case ControlType::TrackBar:
						out << indent << "SendMessageW(" << varName << ", TBM_SETPOS, TRUE, "
							<< ctrl.value << ");\n";
						break;
					case ControlType::UpDown:
						out << indent << "SendMessageW(" << varName << ", UDM_SETPOS32, 0, "
							<< ctrl.value << ");\n";
						break;
					default: break;
					}
				}
			}

			if (!ctrl.children.empty())
			{
				out << "\n";
				auto childIndex = 0;
				EmitControlCreation(out, ctrl.children, varName, childIndex, indent, formFont, fontMap, hasTooltips);
			}

			out << "\n";
			++controlIndex;
		}
	}
}

export namespace FormDesigner
{
	// Generates a standalone C++ source file from a Form definition.
	auto GenerateCode(const Form& form, bool useModules) -> std::string
	{
		auto out = std::ostringstream{};
		auto richEdit = NeedsRichEdit(form.controls);
		auto hasTooltips = NeedsTooltips(form.controls);
		auto className = MakeClassName(form.title);
		auto formStyleExpr = BuildFormStyleExpression(form.style, form.enabled);

		// Check if data binding is needed.
		auto hasBindings = false;
		if (!form.bindStruct.empty())
			for (auto& c : form.controls)
				if (!c.bindField.empty() && BindTypeFor(c.type) != BindType::None)
				{ hasBindings = true; break; }

		// Collect all event dispatches.
		auto dispatch = EventDispatch{};
		for (auto& ctrl : form.controls)
			CollectEvents(ctrl, dispatch);

		// ============================
		// 1. File header / preamble
		// ============================
		out << "// Generated by Win32 Form Builder\n";
		out << "// Form: " << ToNarrow(form.title) << "\n\n";

		if (useModules)
		{
			out << "#define WIN32_LEAN_AND_MEAN\n";
			out << "#include <windows.h>\n";
			out << "#include <commctrl.h>\n";
			if (richEdit)
				out << "#include <richedit.h>\n";
			out << "\n";
			out << "import std;\n\n";
		}
		else
		{
			out << "#define WIN32_LEAN_AND_MEAN\n";
			out << "#include <windows.h>\n";
			out << "#include <commctrl.h>\n";
			if (richEdit)
				out << "#include <richedit.h>\n";
			out << "\n";
		}

		out << "#pragma comment(lib, \"comctl32.lib\")\n";
		if (richEdit)
			out << "#pragma comment(lib, \"msftedit.lib\")\n";
		out << "\n";

		// ============================
		// 2. Resource ID constants
		// ============================
		auto hasIds = false;
		for (auto& ctrl : form.controls)
		{
			if (ctrl.id != 0) { hasIds = true; break; }
			for (auto& child : ctrl.children)
				if (child.id != 0) { hasIds = true; break; }
			if (hasIds) break;
		}

		if (hasIds)
		{
			out << "// Resource IDs\n";
			auto emitIds = [&](auto& self, std::span<const Control> controls) -> void
			{
				for (auto& ctrl : controls)
				{
					if (ctrl.id != 0)
						out << "#define " << IdcConstantName(ctrl) << "  " << ctrl.id << "\n";
					if (!ctrl.children.empty())
						self(self, ctrl.children);
				}
			};
			emitIds(emitIds, form.controls);
			out << "\n";
		}

		// ============================
		// 3. Forward declarations
		// ============================
		out << "// Forward declarations\n";
		out << "LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);\n";
		out << "void CreateFormControls(HWND hwnd, HINSTANCE hInstance);\n";
		if (hasBindings)
		{
			out << "void PopulateForm(HWND hwnd, const " << form.bindStruct << "& data);\n";
			out << "void ReadForm(HWND hwnd, " << form.bindStruct << "& data);\n";
		}
		out << "\n";

		// ============================
		// 4. Event handler stubs
		// ============================
		if (!dispatch.stubs.empty())
		{
			out << "// Event handlers — implement your logic here\n";
			for (auto& [funcName, eventType] : dispatch.stubs)
			{
				if (eventType == "check")
					out << "void " << funcName << "(HWND hwnd, int controlId, bool checked) { /* TODO */ }\n";
				else
					out << "void " << funcName << "(HWND hwnd, int controlId) { /* TODO */ }\n";
			}
			out << "\n";
		}

		// ============================
		// 5. CreateFormControls
		// ============================
		// Collect unique font configurations.
		auto fontMap = std::map<FontKey, std::string>{};
		auto fontIndex = 0;

		// Always include the default/resolved form font.
		auto defaultResolved = ResolveFont(FontInfo{}, form.font);
		auto defaultKey = FontKey{ defaultResolved.family, defaultResolved.size, defaultResolved.bold, defaultResolved.italic };
		fontMap[defaultKey] = "hFont";
		fontIndex = 1;

		CollectFonts(form.controls, form.font, fontMap, fontIndex);

		bool hasCustomFonts = fontMap.size() > 1 || defaultKey.family != DefaultFontFamily
			|| defaultKey.size != DefaultFontSize || defaultKey.bold || defaultKey.italic;

		out << "void CreateFormControls(HWND hwnd, HINSTANCE hInstance)\n";
		out << "{\n";

		if (!hasCustomFonts)
		{
			out << "    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);\n\n";
		}
		else
		{
			// Emit CreateFontW for each unique font.
			for (auto& [key, varName] : fontMap)
			{
				out << "    HFONT " << varName << " = CreateFontW("
					<< "-MulDiv(" << key.size << ", GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72), "
					<< "0, 0, 0, "
					<< (key.bold ? "FW_BOLD" : "FW_NORMAL") << ", "
					<< (key.italic ? "TRUE" : "FALSE") << ", "
					<< "FALSE, FALSE, DEFAULT_CHARSET, "
					<< "OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, "
					<< "DEFAULT_PITCH | FF_DONTCARE, "
					<< "L\"" << ToNarrow(key.family) << "\");\n";
			}
			out << "\n";
		}

		if (hasTooltips)
		{
			out << "    // Create tooltip control\n";
			out << "    HWND hTooltip = CreateWindowExW(0, TOOLTIPS_CLASS, NULL,\n";
			out << "        TTS_ALWAYSTIP | TTS_NOPREFIX, 0, 0, 0, 0,\n";
			out << "        hwnd, NULL, hInstance, NULL);\n";
			out << "    SendMessageW(hTooltip, TTM_SETMAXTIPWIDTH, 0, "
				<< DefaultTooltipWidth << ");\n\n";
		}

		auto controlIndex = 0;
		EmitControlCreation(out, form.controls, "hwnd", controlIndex, "    ", form.font, fontMap, hasTooltips);

		out << "}\n\n";

		// ============================
		// 5b. Data binding helpers
		// ============================
		if (!form.bindStruct.empty())
		{
			// Collect bound controls.
			struct BoundControl { const Control* ctrl; BindType bt; std::string idcName; };
			auto bound = std::vector<BoundControl>{};
			for (auto& ctrl : form.controls)
			{
				if (ctrl.bindField.empty()) continue;
				auto bt = BindTypeFor(ctrl.type);
				if (bt == BindType::None) continue;
				bound.push_back({ &ctrl, bt, IdcConstantName(ctrl) });
			}

			if (!bound.empty())
			{
				// Struct skeleton comment.
				out << "// Suggested struct definition:\n";
				out << "// struct " << form.bindStruct << "\n// {\n";
				for (auto& b : bound)
					out << "//     " << BindTypeName(b.bt) << " " << b.ctrl->bindField << ";\n";
				out << "// };\n\n";

				// PopulateForm.
				out << "void PopulateForm(HWND hwnd, const " << form.bindStruct << "& data)\n";
				out << "{\n";
				for (auto& b : bound)
				{
					switch (b.bt)
					{
					case BindType::Text:
						out << "    SetDlgItemTextW(hwnd, " << b.idcName
							<< ", data." << b.ctrl->bindField << ".c_str());\n";
						break;
					case BindType::Bool:
						out << "    SendDlgItemMessageW(hwnd, " << b.idcName
							<< ", BM_SETCHECK, data." << b.ctrl->bindField
							<< " ? BST_CHECKED : BST_UNCHECKED, 0);\n";
						break;
					case BindType::Int:
						out << "    SetDlgItemInt(hwnd, " << b.idcName
							<< ", static_cast<UINT>(data." << b.ctrl->bindField << "), TRUE);\n";
						break;
					case BindType::Index:
						if (b.ctrl->type == ControlType::ComboBox)
							out << "    SendDlgItemMessageW(hwnd, " << b.idcName
								<< ", CB_SETCURSEL, data." << b.ctrl->bindField << ", 0);\n";
						else
							out << "    SendDlgItemMessageW(hwnd, " << b.idcName
								<< ", LB_SETCURSEL, data." << b.ctrl->bindField << ", 0);\n";
						break;
					default: break;
					}
				}
				out << "}\n\n";

				// ReadForm.
				out << "void ReadForm(HWND hwnd, " << form.bindStruct << "& data)\n";
				out << "{\n";
				for (auto& b : bound)
				{
					switch (b.bt)
					{
					case BindType::Text:
						out << "    {\n";
						out << "        wchar_t buf[1024]{};\n";
						out << "        GetDlgItemTextW(hwnd, " << b.idcName
							<< ", buf, 1024);\n";
						out << "        data." << b.ctrl->bindField << " = buf;\n";
						out << "    }\n";
						break;
					case BindType::Bool:
						out << "    data." << b.ctrl->bindField
							<< " = (SendDlgItemMessageW(hwnd, " << b.idcName
							<< ", BM_GETCHECK, 0, 0) == BST_CHECKED);\n";
						break;
					case BindType::Int:
						out << "    data." << b.ctrl->bindField
							<< " = static_cast<int>(GetDlgItemInt(hwnd, " << b.idcName
							<< ", nullptr, TRUE));\n";
						break;
					case BindType::Index:
						if (b.ctrl->type == ControlType::ComboBox)
							out << "    data." << b.ctrl->bindField
								<< " = static_cast<int>(SendDlgItemMessageW(hwnd, " << b.idcName
								<< ", CB_GETCURSEL, 0, 0));\n";
						else
							out << "    data." << b.ctrl->bindField
								<< " = static_cast<int>(SendDlgItemMessageW(hwnd, " << b.idcName
								<< ", LB_GETCURSEL, 0, 0));\n";
						break;
					default: break;
					}
				}
				out << "}\n\n";
			}
		}

		// ============================
		// 6. WndProc
		// ============================
		out << "LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)\n";
		out << "{\n";
		out << "    switch (msg)\n";
		out << "    {\n";

		// WM_CREATE
		out << "    case WM_CREATE:\n";
		out << "        CreateFormControls(hwnd, ((LPCREATESTRUCT)lParam)->hInstance);\n";
		out << "        return 0;\n\n";

		// WM_COMMAND
		if (!dispatch.command.empty() || !dispatch.checkEntries.empty())
		{
			out << "    case WM_COMMAND:\n";
			out << "    {\n";
			out << "        int id = LOWORD(wParam);\n";
			out << "        int code = HIWORD(wParam);\n";
			if (!dispatch.checkEntries.empty())
				out << "        HWND ctrlHwnd = (HWND)lParam;\n";
			out << "\n";

			for (auto& [notifCode, entries] : dispatch.command)
			{
				out << "        if (code == " << notifCode << ")\n";
				out << "        {\n";
				out << "            switch (id)\n";
				out << "            {\n";
				for (auto& e : entries)
					out << "            case " << e.idcName << ": " << e.functionName << "(hwnd, id); break;\n";
				out << "            }\n";
				out << "        }\n\n";
			}

			// onCheck entries (BN_CLICKED → query check state)
			if (!dispatch.checkEntries.empty())
			{
				out << "        if (code == BN_CLICKED)\n";
				out << "        {\n";
				for (auto& e : dispatch.checkEntries)
				{
					out << "            if (id == " << e.idcName << ")\n";
					out << "                " << e.functionName
						<< "(hwnd, id, SendMessageW(ctrlHwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);\n";
				}
				out << "        }\n\n";
			}

			out << "        return 0;\n";
			out << "    }\n\n";
		}

		// WM_NOTIFY
		if (!dispatch.notify.empty())
		{
			out << "    case WM_NOTIFY:\n";
			out << "    {\n";
			out << "        NMHDR* nmhdr = (NMHDR*)lParam;\n";
			out << "        int id = (int)nmhdr->idFrom;\n\n";

			for (auto& [notifCode, entries] : dispatch.notify)
			{
				out << "        if (nmhdr->code == " << notifCode << ")\n";
				out << "        {\n";
				out << "            switch (id)\n";
				out << "            {\n";
				for (auto& e : entries)
					out << "            case " << e.idcName << ": " << e.functionName << "(hwnd, id); break;\n";
				out << "            }\n";
				out << "        }\n\n";
			}

			out << "        return 0;\n";
			out << "    }\n\n";
		}

		// WM_ERASEBKGND (background color)
		if (form.backgroundColor != -1)
		{
			auto cr = static_cast<unsigned int>(form.backgroundColor);

			out << "    case WM_ERASEBKGND:\n";
			out << "    {\n";
			out << "        static HBRUSH hBrush = CreateSolidBrush(RGB("
				<< (cr & 0xFF) << ", " << ((cr >> 8) & 0xFF) << ", " << ((cr >> 16) & 0xFF) << "));\n";
			out << "        HDC hdc = (HDC)wParam;\n";
			out << "        RECT rc;\n";
			out << "        GetClientRect(hwnd, &rc);\n";
			out << "        FillRect(hdc, &rc, hBrush);\n";
			out << "        return 1;\n";
			out << "    }\n\n";
		}

		// WM_SIZE (anchor-based control repositioning)
		{
			// Collect controls with non-default anchoring.
			struct AnchorEntry { std::string idcName; Rect rect; int anchor; };
			auto anchorEntries = std::vector<AnchorEntry>{};
			auto collectAnchors = [&](auto& self, std::span<const Control> controls) -> void
			{
				for (auto& ctrl : controls)
				{
					if (ctrl.id != 0 && ctrl.anchor != Anchor::Default)
						anchorEntries.push_back({ IdcConstantName(ctrl), ctrl.rect, ctrl.anchor });
					if (!ctrl.children.empty())
						self(self, ctrl.children);
				}
			};
			collectAnchors(collectAnchors, form.controls);

			if (!anchorEntries.empty())
			{
				out << "    case WM_SIZE:\n";
				out << "    {\n";
				out << "        int newW = LOWORD(lParam);\n";
				out << "        int newH = HIWORD(lParam);\n";
				out << "        int deltaW = newW - " << form.width << ";\n";
				out << "        int deltaH = newH - " << form.height << ";\n";
				out << "\n";

				for (auto& ae : anchorEntries)
				{
					bool anchorL = (ae.anchor & Anchor::Left) != 0;
					bool anchorR = (ae.anchor & Anchor::Right) != 0;
					bool anchorT = (ae.anchor & Anchor::Top) != 0;
					bool anchorB = (ae.anchor & Anchor::Bottom) != 0;

					auto x = std::format("MulDiv({}, dpi, 96)", ae.rect.x);
					auto y = std::format("MulDiv({}, dpi, 96)", ae.rect.y);
					auto w = std::format("MulDiv({}, dpi, 96)", ae.rect.width);
					auto h = std::format("MulDiv({}, dpi, 96)", ae.rect.height);

					if (anchorL && anchorR)
						w += " + deltaW";
					else if (anchorR && !anchorL)
						x += " + deltaW";

					if (anchorT && anchorB)
						h += " + deltaH";
					else if (anchorB && !anchorT)
						y += " + deltaH";

					out << "        MoveWindow(GetDlgItem(hwnd, " << ae.idcName << "), "
						<< x << ", " << y << ", " << w << ", " << h << ", TRUE);\n";
				}

				out << "        return 0;\n";
				out << "    }\n\n";
			}
		}

		// WM_DESTROY
		out << "    case WM_DESTROY:\n";
		if (hasCustomFonts)
		{
			out << "    {\n";
			// Note: the font handles are local to CreateFormControls, so we need
			// to generate them as globals or statics. For simplicity, the generated
			// code uses local fonts which are valid for the lifetime of the window.
			// The OS cleans up GDI objects on process exit.
			out << "        PostQuitMessage(0);\n";
			out << "        return 0;\n";
			out << "    }\n";
		}
		else
		{
			out << "        PostQuitMessage(0);\n";
			out << "        return 0;\n";
		}

		out << "    }\n";
		out << "    return DefWindowProcW(hwnd, msg, wParam, lParam);\n";
		out << "}\n\n";

		// ============================
		// 7. WinMain
		// ============================
		out << "int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)\n";
		out << "{\n";
		out << "    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);\n\n";
		out << "    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES };\n";
		out << "    InitCommonControlsEx(&icc);\n\n";

		if (richEdit)
			out << "    LoadLibraryW(L\"msftedit.dll\");\n\n";

		out << "    WNDCLASSEXW wc = {};\n";
		out << "    wc.cbSize = sizeof(wc);\n";
		out << "    wc.style = CS_HREDRAW | CS_VREDRAW;\n";
		out << "    wc.lpfnWndProc = WndProc;\n";
		out << "    wc.hInstance = hInstance;\n";
		out << "    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);\n";
		out << "    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);\n";
		out << "    wc.lpszClassName = L\"" << className << "\";\n";
		out << "    RegisterClassExW(&wc);\n\n";

		out << "    UINT dpi = GetDpiForSystem();\n";
		out << "    RECT rc = { 0, 0, MulDiv(" << form.width << ", dpi, 96), MulDiv(" << form.height << ", dpi, 96) };\n";
		out << "    AdjustWindowRectExForDpi(&rc, " << formStyleExpr << ", FALSE, "
			<< (form.exStyle != 0 ? std::format("0x{:X}", form.exStyle) : "0") << ", dpi);\n\n";

		out << "    HWND hwnd = CreateWindowExW(\n";
		out << "        " << (form.exStyle != 0 ? std::format("0x{:X}", form.exStyle) : "0") << ",\n";
		out << "        L\"" << className << "\",\n";
		out << "        L\"" << EscapeWString(form.title) << "\",\n";
		out << "        " << formStyleExpr << ",\n";
		out << "        CW_USEDEFAULT, CW_USEDEFAULT,\n";
		out << "        rc.right - rc.left, rc.bottom - rc.top,\n";
		out << "        NULL, NULL, hInstance, NULL);\n\n";

		if (form.visible)
		{
			out << "    ShowWindow(hwnd, nCmdShow);\n";
			out << "    UpdateWindow(hwnd);\n\n";
		}
		else
		{
			out << "    // Form starts hidden; call ShowWindow(hwnd, SW_SHOW) when ready.\n\n";
		}

		out << "    MSG msg;\n";
		out << "    while (GetMessageW(&msg, NULL, 0, 0))\n";
		out << "    {\n";
		out << "        TranslateMessage(&msg);\n";
		out << "        DispatchMessageW(&msg);\n";
		out << "    }\n";
		out << "    return (int)msg.wParam;\n";
		out << "}\n";

		return out.str();
	}

	// =========================================================================
	// RC Dialog Export
	// =========================================================================

	// Pixel-to-DLU conversion using font-aware base unit estimation.
	struct DluMetrics
	{
		int baseX = 7;   // Horizontal base unit (avg char width)
		int baseY = 14;  // Vertical base unit (char height)
	};

	auto EstimateDluMetrics(const FontInfo& font) -> DluMetrics
	{
		if (font.family.empty() || font.size == 0)
			return { 7, 14 }; // MS Shell Dlg 2, 8pt defaults

		// Approximate base units proportional to font size.
		// At 8pt: baseX~7, baseY~14. Scale linearly.
		int baseX = std::max(1, static_cast<int>(font.size * 0.875 + 0.5));
		int baseY = std::max(1, static_cast<int>(font.size * 1.75 + 0.5));
		return { baseX, baseY };
	}

	auto PixelToDluX(int px, const DluMetrics& m) -> int
	{
		return ::MulDiv(px, 4, m.baseX);
	}

	auto PixelToDluY(int py, const DluMetrics& m) -> int
	{
		return ::MulDiv(py, 8, m.baseY);
	}

	// Returns the Win32 class name string literal for use in RC CONTROL statements.
	auto RcClassName(ControlType type) -> const char*
	{
		switch (type)
		{
		case ControlType::ProgressBar:       return "msctls_progress32";
		case ControlType::TrackBar:          return "msctls_trackbar32";
		case ControlType::DateTimePicker:    return "SysDateTimePick32";
		case ControlType::TabControl:        return "SysTabControl32";
		case ControlType::ListView:          return "SysListView32";
		case ControlType::TreeView:          return "SysTreeView32";
		case ControlType::UpDown:            return "msctls_updown32";
		case ControlType::RichEdit:          return "RICHEDIT50W";
		case ControlType::MonthCalendar:     return "SysMonthCal32";
		case ControlType::Link:              return "SysLink";
		case ControlType::IPAddress:         return "SysIPAddress32";
		case ControlType::HotKey:            return "msctls_hotkey32";
		case ControlType::Picture:           return "Static";
		case ControlType::Separator:         return "Static";
		case ControlType::Animation:         return "SysAnimate32";
		default:                             return "Static";
		}
	}

	// Builds a style expression for use in RC scripts.
	auto BuildRcStyleExpression(ControlType type, TextAlign align, Win32::DWORD customStyle, bool forGenericControl, bool visible = true, bool enabled = true, bool tabStop = true, bool groupStart = false) -> std::string
	{
		auto parts = std::vector<std::string>{};

		if (forGenericControl)
		{
			// Generic CONTROL statements need WS_CHILD | WS_VISIBLE | WS_TABSTOP explicitly.
			// The resource compiler doesn't add them automatically for CONTROL.
		}

		// For keyword controls (non-generic), use NOT WS_VISIBLE to suppress visibility.
		// For generic controls, the visibility is handled via the numeric base style.
		if (!visible && !forGenericControl)
			parts.push_back("NOT WS_VISIBLE");

		if (!enabled)
			parts.push_back("WS_DISABLED");

		if (tabStop && IsInteractiveControl(type) && !forGenericControl)
			parts.push_back("WS_TABSTOP");

		if (groupStart)
			parts.push_back("WS_GROUP");

		switch (type)
		{
		case ControlType::TextBox:     parts.push_back("ES_AUTOHSCROLL"); break;
		case ControlType::ListBox:     parts.push_back("LBS_STANDARD"); break;
		case ControlType::ComboBox:    parts.push_back("CBS_DROPDOWNLIST"); parts.push_back("WS_VSCROLL"); break;
		case ControlType::ListView:    parts.push_back("LVS_REPORT"); parts.push_back("LVS_SHOWSELALWAYS"); break;
		case ControlType::TreeView:    parts.push_back("TVS_HASBUTTONS"); parts.push_back("TVS_HASLINES"); parts.push_back("TVS_LINESATROOT"); break;
		case ControlType::RichEdit:    parts.push_back("ES_MULTILINE"); parts.push_back("ES_AUTOVSCROLL"); parts.push_back("WS_BORDER"); break;
		case ControlType::Picture:     parts.push_back("SS_ETCHEDFRAME"); break;
		case ControlType::Separator:   parts.push_back("SS_ETCHEDHORZ"); break;
		default: break;
		}

		switch (type)
		{
		case ControlType::Label:
			switch (align)
			{
			case TextAlign::Center: parts.push_back("SS_CENTER"); break;
			case TextAlign::Right:  parts.push_back("SS_RIGHT"); break;
			default: break;
			}
			break;
		case ControlType::TextBox:
		case ControlType::RichEdit:
			switch (align)
			{
			case TextAlign::Center: parts.push_back("ES_CENTER"); break;
			case TextAlign::Right:  parts.push_back("ES_RIGHT"); break;
			default: break;
			}
			break;
		case ControlType::Button:
			switch (align)
			{
			case TextAlign::Left:   parts.push_back("BS_LEFT"); break;
			case TextAlign::Center: parts.push_back("BS_CENTER"); break;
			case TextAlign::Right:  parts.push_back("BS_RIGHT"); break;
			}
			break;
		default: break;
		}

		if (customStyle != 0)
			parts.push_back(std::format("0x{:X}", customStyle));

		if (parts.empty())
		{
			if (!forGenericControl) return "";
			// Base styles for generic CONTROL: WS_CHILD always present.
			// WS_CHILD = 0x40000000, WS_VISIBLE = 0x10000000, WS_TABSTOP = 0x00010000
			auto base = Win32::DWORD{0x40000000}; // WS_CHILD
			if (visible) base |= 0x10000000;      // WS_VISIBLE
			if (tabStop && IsInteractiveControl(type)) base |= 0x00010000; // WS_TABSTOP
			if (groupStart) base |= 0x00020000;    // WS_GROUP
			return std::format("0x{:08X}", base);
		}

		auto result = std::string{};
		for (size_t i = 0; i < parts.size(); ++i)
		{
			if (i > 0) result += " | ";
			result += parts[i];
		}
		return result;
	}

	// Escapes a wide string to narrow for use in RC string literals.
	auto EscapeRcString(const std::wstring& s) -> std::string
	{
		auto result = std::string{};
		for (auto ch : s)
		{
			if (ch == L'"') result += "\"\"";
			else if (ch < 128) result += static_cast<char>(ch);
			else result += std::format("\\x{:04X}", static_cast<unsigned int>(ch));
		}
		return result;
	}

	// Generates a resource.h header with IDD and IDC defines.
	auto GenerateRcHeader(const Form& form) -> std::string
	{
		auto out = std::ostringstream{};

		out << "//{{NO_DEPENDENCIES}}\n";
		out << "// Resource header generated by Win32 Form Builder\n";
		out << "#ifndef RESOURCE_H\n";
		out << "#define RESOURCE_H\n\n";

		out << "#define IDD_DIALOG  101\n\n";

		// Collect all controls with IDs.
		auto emitDefines = [&](auto& self, std::span<const Control> controls) -> void
		{
			for (auto& ctrl : controls)
			{
				if (ctrl.id > 0)
				{
					auto prefix = TypePrefixUpper(ctrl.type);
					out << "#define IDC_" << prefix << "_" << ctrl.id
						<< "  " << ctrl.id << "\n";
				}
				if (!ctrl.children.empty())
					self(self, ctrl.children);
			}
		};
		emitDefines(emitDefines, form.controls);

		out << "\n#endif // RESOURCE_H\n";
		return out.str();
	}

	// Generates a C++ header with named control ID constants.
	// Produces a namespace with inline constexpr int constants derived from
	// control text (PascalCase) and type, for use with FormWindow::Get<T>(id).
	auto GenerateControlIds(const Form& form) -> std::string
	{
		auto out = std::ostringstream{};

		out << "// Control ID constants generated by Win32 Form Builder\n";
		out << "// Use with FormDesigner::FormWindow::Get<T>(id)\n";
		out << "#pragma once\n\n";

		out << "namespace Controls\n{\n";

		// Collect all controls and build names, resolving duplicates.
		struct IdEntry { std::string name; int id; };
		auto entries = std::vector<IdEntry>{};

		auto collect = [&](auto& self, std::span<const Control> controls) -> void
		{
			for (auto& ctrl : controls)
			{
				if (ctrl.id > 0)
					entries.push_back({BuildControlIdName(ctrl), ctrl.id});
				if (!ctrl.children.empty())
					self(self, ctrl.children);
			}
		};
		collect(collect, form.controls);

		// Detect duplicate names and append ID to disambiguate.
		auto nameCounts = std::unordered_map<std::string, int>{};
		for (auto& e : entries)
			nameCounts[e.name]++;
		for (auto& e : entries)
		{
			if (nameCounts[e.name] > 1)
				e.name += std::to_string(e.id);
		}

		for (auto& e : entries)
			out << "    inline constexpr int " << e.name << " = " << e.id << ";\n";

		out << "}\n";
		return out.str();
	}

	// Emits a single control line in RC DIALOGEX format.
	void EmitRcControl(std::ostringstream& out, const Control& ctrl,
		const DluMetrics& metrics)
	{
		// Emit image path comment for Picture controls.
		if (ctrl.type == ControlType::Picture && !ctrl.imagePath.empty())
			out << "    // Image: " << ToNarrow(ctrl.imagePath) << "\n";

		int x = PixelToDluX(ctrl.rect.x, metrics);
		int y = PixelToDluY(ctrl.rect.y, metrics);
		int w = PixelToDluX(ctrl.rect.width, metrics);
		int h = PixelToDluY(ctrl.rect.height, metrics);

		auto text = EscapeRcString(ctrl.text);
		auto idName = ctrl.id > 0
			? std::format("IDC_{}_{}", TypePrefixUpper(ctrl.type), ctrl.id)
			: std::string{"IDC_STATIC"};

		bool isDefPush = (ctrl.type == ControlType::Button && (ctrl.style & 0x1) != 0);

		auto styleStr = BuildRcStyleExpression(ctrl.type, ctrl.textAlign, ctrl.style, false, ctrl.visible, ctrl.enabled, ctrl.tabStop, ctrl.groupStart);

		switch (ctrl.type)
		{
		case ControlType::Label:
		{
			auto keyword = "LTEXT";
			if (ctrl.textAlign == TextAlign::Center) keyword = "CTEXT";
			else if (ctrl.textAlign == TextAlign::Right) keyword = "RTEXT";
			out << "    " << keyword << "           \""
				<< text << "\"," << idName << ","
				<< x << "," << y << "," << w << "," << h;
			if (!styleStr.empty())
				out << "," << styleStr;
			out << "\n";
			break;
		}
		case ControlType::Button:
			out << "    " << (isDefPush ? "DEFPUSHBUTTON" : "PUSHBUTTON")
				<< "      \"" << text << "\"," << idName << ","
				<< x << "," << y << "," << w << "," << h;
			if (!styleStr.empty())
				out << "," << styleStr;
			out << "\n";
			break;
		case ControlType::CheckBox:
			out << "    AUTOCHECKBOX    \""
				<< text << "\"," << idName << ","
				<< x << "," << y << "," << w << "," << h;
			if (!styleStr.empty())
				out << "," << styleStr;
			out << "\n";
			break;
		case ControlType::RadioButton:
			out << "    AUTORADIOBUTTON \""
				<< text << "\"," << idName << ","
				<< x << "," << y << "," << w << "," << h;
			if (!styleStr.empty())
				out << "," << styleStr;
			out << "\n";
			break;
		case ControlType::GroupBox:
			out << "    GROUPBOX        \""
				<< text << "\"," << idName << ","
				<< x << "," << y << "," << w << "," << h;
			if (!styleStr.empty())
				out << "," << styleStr;
			out << "\n";
			break;
		case ControlType::TextBox:
		{
			out << "    EDITTEXT        "
				<< idName << ","
				<< x << "," << y << "," << w << "," << h;
			if (!styleStr.empty())
				out << "," << styleStr;
			out << "\n";
			break;
		}
		case ControlType::ComboBox:
		{
			// ComboBox height in RC is the dropdown height.
			int dropH = PixelToDluY(200, metrics);
			out << "    COMBOBOX        "
				<< idName << ","
				<< x << "," << y << "," << w << "," << dropH;
			if (!styleStr.empty())
				out << "," << styleStr;
			out << "\n";
			break;
		}
		case ControlType::ListBox:
			out << "    LISTBOX         "
				<< idName << ","
				<< x << "," << y << "," << w << "," << h;
			if (!styleStr.empty())
				out << "," << styleStr;
			out << "\n";
			break;
		default:
		{
			// Generic CONTROL statement for common controls.
			auto className = RcClassName(ctrl.type);
			auto genStyle = BuildRcStyleExpression(ctrl.type, ctrl.textAlign, ctrl.style, true, ctrl.visible, ctrl.enabled, ctrl.tabStop, ctrl.groupStart);
			out << "    CONTROL         \""
				<< text << "\"," << idName << ",\""
				<< className << "\"," << genStyle << ","
				<< x << "," << y << "," << w << "," << h << "\n";
			break;
		}
		}
	}

	// Generates a complete .rc DIALOGEX resource from a Form definition.
	auto GenerateRcDialog(const Form& form) -> std::string
	{
		auto out = std::ostringstream{};

		auto metrics = EstimateDluMetrics(form.font);
		int dlgW = PixelToDluX(form.width, metrics);
		int dlgH = PixelToDluY(form.height, metrics);

		out << "// Resource script generated by Win32 Form Builder\n";
		out << "#include <windows.h>\n";
		out << "#include <commctrl.h>\n";
		out << "#include \"resource.h\"\n\n";

		out << "IDD_DIALOG DIALOGEX 0, 0, " << dlgW << ", " << dlgH << "\n";

		// STYLE
		auto style = form.style != 0 ? form.style : 0x90C80080; // WS_OVERLAPPEDWINDOW | DS_SETFONT | WS_POPUP | DS_MODALFRAME
		if (!form.enabled)
			style |= 0x08000000; // WS_DISABLED
		if (!form.visible)
			style &= ~0x10000000; // Remove WS_VISIBLE if present
		out << "STYLE " << std::format("0x{:08X}", style | 0x40) << "\n"; // Ensure DS_SETFONT

		if (form.exStyle != 0)
			out << "EXSTYLE " << std::format("0x{:08X}", form.exStyle) << "\n";

		out << "CAPTION \"" << EscapeRcString(form.title) << "\"\n";

		// FONT statement
		if (!form.font.family.empty() && form.font.size > 0)
		{
			int weight = form.font.bold ? 700 : 400;
			int italic = form.font.italic ? 1 : 0;
			out << "FONT " << form.font.size << ", \""
				<< EscapeRcString(form.font.family) << "\", "
				<< weight << ", " << italic << ", 0x1\n";
		}
		else
		{
			out << "FONT 8, \"MS Shell Dlg 2\", 400, 0, 0x1\n";
		}

		out << "BEGIN\n";

		auto emitControls = [&](auto& self, std::span<const Control> controls) -> void
		{
			for (auto& ctrl : controls)
			{
				EmitRcControl(out, ctrl, metrics);
				if (!ctrl.children.empty())
					self(self, ctrl.children);
			}
		};
		emitControls(emitControls, form.controls);

		out << "END\n";

		return out.str();
	}
}
