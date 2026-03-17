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

// Build a simple confirmation dialog form programmatically.
auto MakeConfirmDialog(const std::wstring& name, const std::wstring& email) -> FormDesigner::Form
{
	auto form = FormDesigner::Form{};
	form.title = L"Confirm Submission";
	form.width = 320;
	form.height = 160;
	form.style = 0x00C80000; // WS_CAPTION | WS_SYSMENU

	auto label = FormDesigner::Control{};
	label.type = FormDesigner::ControlType::Label;
	label.text = L"Submit form for " + name + L" (" + email + L")?";
	label.rect = { 20, 20, 280, 40 };

	auto okBtn = FormDesigner::Control{};
	okBtn.type = FormDesigner::ControlType::Button;
	okBtn.text = L"OK";
	okBtn.id = 1;
	okBtn.rect = { 60, 80, 80, 30 };
	okBtn.style = 1; // BS_DEFPUSHBUTTON

	auto cancelBtn = FormDesigner::Control{};
	cancelBtn.type = FormDesigner::ControlType::Button;
	cancelBtn.text = L"Cancel";
	cancelBtn.id = 2;
	cancelBtn.rect = { 170, 80, 80, 30 };

	form.controls = { label, okBtn, cancelBtn };
	return form;
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

	auto events = FormDesigner::EventMap{};

	// Submit button: read form values using typed wrappers, show modal confirmation.
	events.onClick(Controls::SubmitButton, [&](const FormDesigner::ClickEvent& e) {
		auto window = FormDesigner::FormWindow{e.formHwnd};

		auto name = window.GetTextBox(Controls::NameText).GetText();
		auto email = window.GetTextBox(Controls::EmailText).GetText();
		auto subscribed = window.GetCheckBox(Controls::SubscribeCheck).IsChecked();

		// Show a modal confirmation dialog.
		auto dialogForm = MakeConfirmDialog(name, email);
		auto dlgEvents = FormDesigner::EventMap{};
		dlgEvents.onClick(1, [](const FormDesigner::ClickEvent& de) {
			FormDesigner::EndModal(de.formHwnd, FormDesigner::DialogResult::Ok);
		});
		dlgEvents.onClick(2, [](const FormDesigner::ClickEvent& de) {
			FormDesigner::EndModal(de.formHwnd, FormDesigner::DialogResult::Cancel);
		});

		auto result = FormDesigner::ShowModalForm(dialogForm, Win32::GetModuleHandleW(nullptr),
			dlgEvents, e.formHwnd);

		if (result == FormDesigner::DialogResult::Ok)
		{
			auto msg = L"Submitted: " + name + L"\nEmail: " + email
				+ L"\nSubscribed: " + (subscribed ? L"Yes" : L"No");
			Win32::MessageBoxW(e.formHwnd, msg.c_str(), L"Success", Win32::Mb_Ok | Win32::Mb_IconInformation);
		}
	});

	// Cancel button: close the form.
	events.onClick(Controls::CancelButton, [](const FormDesigner::ClickEvent& e) {
		Win32::DestroyWindow(e.formHwnd);
	});

	auto hwnd = FormDesigner::LoadForm(form, hInstance, events, formBasePath);

	// Enable hot reload: the form will automatically update when the JSON file is saved.
	FormDesigner::EnableHotReload(hwnd, path, formBasePath);

	return FormDesigner::RunMessageLoop();
}
catch (const std::exception& ex)
{
	auto msg = std::string{ "Failed to load form:\n" } + ex.what();
	auto wide = std::wstring(msg.begin(), msg.end());
	Win32::MessageBoxW(nullptr, wide.c_str(), L"Form Designer Error", Win32::Mb_IconError | Win32::Mb_Ok);
	return 1;
}
