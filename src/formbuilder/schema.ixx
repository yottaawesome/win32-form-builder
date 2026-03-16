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
	};

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
		std::vector<Control> children;
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
		std::vector<Control> controls;
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
		case ControlType::Label:       return Win32::Styles::StaticLeft;
		case ControlType::ListBox:     return Win32::Styles::ListBoxStandard;
		case ControlType::ComboBox:    return Win32::Styles::ComboBoxDropDownList;
		case ControlType::ListView:    return Win32::Styles::ListViewReport | Win32::Styles::ListViewShowSelAlways;
		case ControlType::TreeView:    return Win32::Styles::TreeViewHasButtons | Win32::Styles::TreeViewHasLines | Win32::Styles::TreeViewLinesAtRoot;
		case ControlType::RichEdit:    return Win32::Styles::Border | Win32::Styles::EditMultiLine | Win32::Styles::EditAutoVScroll;
		default:                    return 0;
		}
	}
}
