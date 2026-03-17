export module formbuilder:messagebox;
import :win32;
import :schema;

export namespace FormDesigner
{
	// Simple notification dialogs.

	void ShowInfo(Win32::HWND parent, std::wstring_view message,
		std::wstring_view title = L"Information")
	{
		Win32::MessageBoxW(parent, std::wstring{message}.c_str(),
			std::wstring{title}.c_str(), Win32::Mb_Ok | Win32::Mb_IconInformation);
	}

	void ShowError(Win32::HWND parent, std::wstring_view message,
		std::wstring_view title = L"Error")
	{
		Win32::MessageBoxW(parent, std::wstring{message}.c_str(),
			std::wstring{title}.c_str(), Win32::Mb_Ok | Win32::Mb_IconError);
	}

	void ShowWarning(Win32::HWND parent, std::wstring_view message,
		std::wstring_view title = L"Warning")
	{
		Win32::MessageBoxW(parent, std::wstring{message}.c_str(),
			std::wstring{title}.c_str(), Win32::Mb_Ok | Win32::Mb_IconWarning);
	}

	// Prompt dialogs returning DialogResult.

	auto AskYesNo(Win32::HWND parent, std::wstring_view message,
		std::wstring_view title = L"Confirm") -> DialogResult
	{
		auto result = Win32::MessageBoxW(parent, std::wstring{message}.c_str(),
			std::wstring{title}.c_str(), Win32::Mb_YesNo | Win32::Mb_IconQuestion);
		return static_cast<DialogResult>(result);
	}

	auto AskYesNoCancel(Win32::HWND parent, std::wstring_view message,
		std::wstring_view title = L"Confirm") -> DialogResult
	{
		auto result = Win32::MessageBoxW(parent, std::wstring{message}.c_str(),
			std::wstring{title}.c_str(), Win32::Mb_YesNoCancel | Win32::Mb_IconQuestion);
		return static_cast<DialogResult>(result);
	}

	auto AskOkCancel(Win32::HWND parent, std::wstring_view message,
		std::wstring_view title = L"Confirm") -> DialogResult
	{
		auto result = Win32::MessageBoxW(parent, std::wstring{message}.c_str(),
			std::wstring{title}.c_str(), Win32::Mb_OkCancel | Win32::Mb_IconQuestion);
		return static_cast<DialogResult>(result);
	}
}
