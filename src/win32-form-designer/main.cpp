#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


import std;
import formbuilder;
import designer;

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

	auto form = FormDesigner::Form{};
	auto path = std::filesystem::path{};
	if (lpCmdLine and lpCmdLine[0] != L'\0')
	{
		path = lpCmdLine;
		form = FormDesigner::LoadFormFromFile(path);
	}

	auto hwnd = Designer::CreateDesignSurface(hInstance, std::move(form), std::move(path));
	if (not hwnd)
		return 1;

	return Designer::RunDesignerLoop(hwnd);
}
catch (const std::exception& ex)
{
	auto msg = std::string{ "Failed to start designer:\n" } + ex.what();
	auto wide = std::wstring(msg.begin(), msg.end());
	FormDesigner::ShowError(nullptr, wide, L"Form Designer Error");
	return 1;
}
