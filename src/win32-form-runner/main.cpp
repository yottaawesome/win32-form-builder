#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


import std;
import formbuilder;

// Named control IDs (normally generated via File > Export Control IDs).
namespace Controls
{
	inline constexpr int NameText = 101;
	inline constexpr int EmailText = 102;
	inline constexpr int SubscribeCheck = 201;
	inline constexpr int AcceptCheck = 202;
	inline constexpr int SubmitButton = 301;
	inline constexpr int CancelButton = 302;
}

auto wWinMain(Win32::HINSTANCE hInstance, Win32::HINSTANCE, Win32::LPWSTR lpCmdLine, int) -> int
try
{
	Win32::SetProcessDpiAwarenessContext(Win32::DpiContextPerMonitorAwareV2);

	auto icc = Win32::INITCOMMONCONTROLSEX{
		.dwSize = sizeof(Win32::INITCOMMONCONTROLSEX),
		.dwICC = Win32::Icc_StandardClasses,
	};
	Win32::InitCommonControlsEx(&icc);
	Win32::LoadLibraryW(L"msftedit.dll");

	auto path = std::filesystem::path{};
	if (lpCmdLine and lpCmdLine[0] != L'\0')
	{
		path = lpCmdLine;
	}
	else
	{
		path = L"sample-form.json";
	}

	auto form = FormDesigner::LoadFormFromFile(path);

	// Extract directory from form file path for resolving relative image paths.
	auto formBasePath = std::wstring{};
	{
		auto wpath = std::wstring(path);
		auto sep = wpath.find_last_of(L"\\/");
		if (sep != std::wstring::npos)
			formBasePath = wpath.substr(0, sep);
	}

	// Register submit handler via EventMap (before LoadForm).
	auto events = FormDesigner::EventMap{};
	events.onClick(Controls::SubmitButton, [&](const FormDesigner::ClickEvent& e) {
		auto window = FormDesigner::FormWindow{e.formHwnd};

		auto name = window.GetTextBox(Controls::NameText).GetText();
		auto email = window.GetTextBox(Controls::EmailText).GetText();
		auto subscribed = window.GetCheckBox(Controls::SubscribeCheck).IsChecked();

		// Prompt for confirmation using message box helper.
		auto result = FormDesigner::AskYesNo(e.formHwnd,
			L"Submit form for " + name + L" (" + email + L")?",
			L"Confirm Submission");

		if (result == FormDesigner::DialogResult::Yes)
		{
			auto msg = L"Submitted: " + name + L"\nEmail: " + email
				+ L"\nSubscribed: " + (subscribed ? L"Yes" : L"No");
			FormDesigner::ShowInfo(e.formHwnd, msg, L"Success");
		}
	});

	// Load the form and get a typed window wrapper.
	auto hwnd = FormDesigner::LoadForm(form, hInstance, events, formBasePath);
	auto window = FormDesigner::FormWindow{hwnd};

	// Register cancel handler via wrapper-based event binding (after LoadForm).
	window.GetButton(Controls::CancelButton).OnClick([](const FormDesigner::ClickEvent& e) {
		Win32::DestroyWindow(e.formHwnd);
	});

	// Enable hot reload: the form automatically updates when the JSON file is saved.
	FormDesigner::EnableHotReload(hwnd, path, formBasePath);

	return FormDesigner::RunMessageLoop();
}
catch (const FormDesigner::FormException& ex)
{
	auto msg = std::string{ "Failed to load form:\n" } + ex.what();
	auto wide = std::wstring(msg.begin(), msg.end());
	FormDesigner::ShowError(nullptr, wide, L"Form Runner Error");
	return 1;
}
catch (const std::exception& ex)
{
	auto msg = std::string{ "Unexpected error:\n" } + ex.what();
	auto wide = std::wstring(msg.begin(), msg.end());
	FormDesigner::ShowError(nullptr, wide, L"Form Runner Error");
	return 1;
}
