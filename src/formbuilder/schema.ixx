export module formbuilder:schema;
import std;
import :win32;

export namespace FormDesigner
{
	struct Rect
	{
		int x = 0;
		int y = 0;
		int width = 100;
		int height = 25;
	};

	// Maps to a subset of window styles relevant for form controls.
	enum class ControlType
	{
		Window,     // Top-level or child window
		Button,
		CheckBox,
		RadioButton,
		Label,      // STATIC control
		TextBox,    // EDIT control
		GroupBox,
		ListBox,
		ComboBox,
		ProgressBar,
		TrackBar,
		DateTimePicker,
		TabControl,
		ListView,
		TreeView,
		UpDown,
		RichEdit,
		MonthCalendar,
		Link,       // SysLink control
		IPAddress,
		HotKey,
		Picture,    // STATIC with SS_ETCHEDFRAME
		Separator,  // STATIC with SS_ETCHEDHORZ
		Animation,
	};

	// Text alignment options for controls that support it.
	enum class TextAlign { Left, Center, Right };

	// Font properties for controls and forms.
	struct FontInfo
	{
		std::wstring family;
		int size = 0;
		bool bold = false;
		bool italic = false;

		auto isSet() const noexcept -> bool { return !family.empty() || size != 0; }
	};

	// System default font used when no font is specified.
	inline constexpr int DefaultFontSize = 9;
	inline const std::wstring DefaultFontFamily = L"Segoe UI";

	// Resolves the effective font for a control, cascading: control → form → system default.
	inline auto ResolveFont(const FontInfo& controlFont, const FontInfo& formFont) -> FontInfo
	{
		auto result = FontInfo{
			.family = DefaultFontFamily,
			.size = DefaultFontSize,
			.bold = false,
			.italic = false,
		};

		if (formFont.isSet())
		{
			if (!formFont.family.empty()) result.family = formFont.family;
			if (formFont.size != 0) result.size = formFont.size;
			result.bold = formFont.bold;
			result.italic = formFont.italic;
		}

		if (controlFont.isSet())
		{
			if (!controlFont.family.empty()) result.family = controlFont.family;
			if (controlFont.size != 0) result.size = controlFont.size;
			result.bold = controlFont.bold;
			result.italic = controlFont.italic;
		}

		return result;
	}

	// Anchor flags — bitmask controlling how controls respond to parent resize.
	namespace Anchor
	{
		constexpr int Top    = 1;
		constexpr int Bottom = 2;
		constexpr int Left   = 4;
		constexpr int Right  = 8;
		constexpr int Default = Top | Left;
	}

	// Validation metadata for controls (used by code generators, not enforced at runtime).
	struct ValidationInfo
	{
		bool required = false;
		int minLength = 0;   // 0 = unset
		int maxLength = 0;   // 0 = unset
		std::string pattern; // regex pattern, empty = unset
		int min = 0;         // 0 = unset (for numeric range)
		int max = 0;         // 0 = unset (for numeric range)

		auto isSet() const noexcept -> bool
		{
			return required || minLength != 0 || maxLength != 0
				|| !pattern.empty() || min != 0 || max != 0;
		}
	};

	// Returns true if the control type supports text-length validation fields.
	constexpr auto SupportsTextValidation(ControlType type) noexcept -> bool
	{
		return type == ControlType::TextBox || type == ControlType::RichEdit;
	}

	// Returns true if the control type supports numeric range validation fields.
	constexpr auto SupportsRangeValidation(ControlType type) noexcept -> bool
	{
		return type == ControlType::TrackBar || type == ControlType::UpDown
			|| type == ControlType::ProgressBar;
	}

	// Returns true if the control type supports the "required" validation field.
	constexpr auto SupportsRequiredValidation(ControlType type) noexcept -> bool
	{
		return type == ControlType::TextBox || type == ControlType::ComboBox
			|| type == ControlType::ListBox || type == ControlType::RichEdit
			|| type == ControlType::DateTimePicker;
	}

	struct Control
	{
		ControlType type = ControlType::Window;
		std::wstring text;
		Rect rect;
		int id = 0;
		Win32::DWORD style = 0;
		Win32::DWORD exStyle = 0;
		std::string onClick;
		std::string onChange;
		std::string onDoubleClick;
		std::string onSelectionChange;
		std::string onFocus;
		std::string onBlur;
		std::string onCheck;
		int tabIndex = 0;
		TextAlign textAlign = TextAlign::Left;
		bool visible = true;
		bool enabled = true;
		bool locked = false;
		int groupId = 0;
		int anchor = Anchor::Default;
		FontInfo font;
		std::wstring tooltip;
		std::vector<std::wstring> items;
		int selectedIndex = -1;
		ValidationInfo validation;
		std::wstring imagePath; // For Picture controls: relative path to BMP/ICO file.
		std::string bindField; // Data binding: struct member name.
		bool tabStop = true;     // Whether control receives WS_TABSTOP (interactive controls only).
		bool groupStart = false; // Whether control starts a new WS_GROUP.
		std::wstring accessibleName;        // Explicit accessible name for screen readers.
		std::wstring accessibleDescription; // Accessible description/help text.
		std::vector<Control> children;
	};

	// Returns true if the control type is interactive (receives keyboard focus/input).
	constexpr auto IsInteractiveControl(ControlType type) noexcept -> bool
	{
		switch (type)
		{
		case ControlType::Button:
		case ControlType::CheckBox:
		case ControlType::RadioButton:
		case ControlType::TextBox:
		case ControlType::ListBox:
		case ControlType::ComboBox:
		case ControlType::TreeView:
		case ControlType::ListView:
		case ControlType::DateTimePicker:
		case ControlType::MonthCalendar:
		case ControlType::HotKey:
		case ControlType::IPAddress:
		case ControlType::RichEdit:
		case ControlType::TrackBar:
		case ControlType::UpDown:
		case ControlType::TabControl:
			return true;
		default:
			return false;
		}
	}

	// Returns 0 for unknown, 1 for BMP (IMAGE_BITMAP), 2 for ICO (IMAGE_ICON).
	inline auto ImageTypeFromPath(const std::wstring& path) noexcept -> int
	{
		if (path.size() < 4) return 0;
		auto ext = path.substr(path.size() - 4);
		// Case-insensitive comparison.
		for (auto& c : ext) c = (c >= L'A' && c <= L'Z') ? (c + 32) : c;
		if (ext == L".bmp") return 1;
		if (ext == L".ico") return 2;
		return 0;
	}

	// A designer guide line (persisted with the form).
	struct DesignerGuide
	{
		bool horizontal = false;
		int position = 0;
	};

	// A complete form definition: a top-level window with child controls.
	struct Form
	{
		std::wstring title = L"Untitled";
		int width = 640;
		int height = 480;
		Win32::DWORD style = Win32::Styles::OverlappedWindow;
		Win32::DWORD exStyle = 0;
		int backgroundColor = -1; // -1 = system default; otherwise COLORREF
		FontInfo font;
		std::string bindStruct; // Data binding: C++ struct name for PopulateForm/ReadForm.
		std::vector<Control> controls;
		std::vector<DesignerGuide> guides;
	};

	// Returns the Win32 window class name for a given ControlType.
	constexpr auto ClassNameFor(ControlType type) noexcept -> const wchar_t*
	{
		switch (type)
		{
		case ControlType::Button:      return Win32::Controls::Button;
		case ControlType::CheckBox:    return Win32::Controls::Button;
		case ControlType::RadioButton: return Win32::Controls::Button;
		case ControlType::Label:       return Win32::Controls::Static;
		case ControlType::TextBox:     return Win32::Controls::Edit;
		case ControlType::GroupBox:    return Win32::Controls::Button;
		case ControlType::ListBox:     return Win32::Controls::ListBox;
		case ControlType::ComboBox:    return Win32::Controls::ComboBox;
		case ControlType::ProgressBar:     return Win32::Controls::Progress;
		case ControlType::TrackBar:        return Win32::Controls::TrackBar;
		case ControlType::DateTimePicker:  return Win32::Controls::DateTimePick;
		case ControlType::TabControl:      return Win32::Controls::Tab;
		case ControlType::ListView:        return Win32::Controls::ListView;
		case ControlType::TreeView:        return Win32::Controls::TreeView;
		case ControlType::UpDown:          return Win32::Controls::UpDown;
		case ControlType::RichEdit:        return Win32::Controls::RichEdit;
		case ControlType::MonthCalendar:   return Win32::Controls::MonthCalendar;
		case ControlType::Link:            return Win32::Controls::Link;
		case ControlType::IPAddress:       return Win32::Controls::IPAddress;
		case ControlType::HotKey:          return Win32::Controls::HotKey;
		case ControlType::Picture:         return Win32::Controls::Static;
		case ControlType::Separator:       return Win32::Controls::Static;
		case ControlType::Animation:       return Win32::Controls::Animation;
		default:                    return nullptr;
		}
	}

	// Returns additional style flags implied by the ControlType.
	constexpr auto ImpliedStyleFor(ControlType type) noexcept -> Win32::DWORD
	{
		switch (type)
		{
		case ControlType::CheckBox:    return Win32::Styles::AutoCheckBox;
		case ControlType::RadioButton: return Win32::Styles::AutoRadioButton;
		case ControlType::GroupBox:    return Win32::Styles::GroupBox;
		case ControlType::TextBox:     return Win32::Styles::Border | Win32::Styles::EditAutoHScroll;
		case ControlType::Label:       return 0; // Alignment applied via AlignmentStyleFor
		case ControlType::ListBox:     return Win32::Styles::ListBoxStandard;
		case ControlType::ComboBox:    return Win32::Styles::ComboBoxDropDownList;
		case ControlType::ListView:    return Win32::Styles::ListViewReport | Win32::Styles::ListViewShowSelAlways;
		case ControlType::TreeView:    return Win32::Styles::TreeViewHasButtons | Win32::Styles::TreeViewHasLines | Win32::Styles::TreeViewLinesAtRoot;
		case ControlType::RichEdit:    return Win32::Styles::Border | Win32::Styles::EditMultiLine | Win32::Styles::EditAutoVScroll;
		case ControlType::Picture:     return Win32::Styles::StaticEtchedFrame;
		case ControlType::Separator:   return Win32::Styles::StaticEtchedHorz;
		default:                    return 0;
		}
	}

	// Returns the Win32 style bits for text alignment on a given control type.
	export constexpr auto AlignmentStyleFor(ControlType type, TextAlign align) noexcept -> Win32::DWORD
	{
		switch (type)
		{
		case ControlType::Label:
			switch (align)
			{
			case TextAlign::Left:   return Win32::Styles::StaticLeft;
			case TextAlign::Center: return Win32::Styles::StaticCenter;
			case TextAlign::Right:  return Win32::Styles::StaticRight;
			}
			break;
		case ControlType::TextBox:
		case ControlType::RichEdit:
			switch (align)
			{
			case TextAlign::Left:   return 0; // ES_LEFT is 0
			case TextAlign::Center: return Win32::Styles::EditCenter;
			case TextAlign::Right:  return Win32::Styles::EditRight;
			}
			break;
		case ControlType::Button:
		case ControlType::CheckBox:
		case ControlType::RadioButton:
			switch (align)
			{
			case TextAlign::Left:   return Win32::Styles::ButtonLeft;
			case TextAlign::Center: return Win32::Styles::ButtonCenter;
			case TextAlign::Right:  return Win32::Styles::ButtonRight;
			}
			break;
		default:
			break;
		}
		return 0;
	}
}
