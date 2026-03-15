#pragma comment(lib, "Comctl32.lib")

import std;
import formbuilder;
import designer;

auto __stdcall wWinMain(Win32::HINSTANCE hInstance, Win32::HINSTANCE, Win32::LPWSTR lpCmdLine, int) -> int
try
{
	auto icc = Win32::INITCOMMONCONTROLSEX{
		.dwSize = sizeof(Win32::INITCOMMONCONTROLSEX),
		.dwICC = Win32::Icc_StandardClasses,
	};
	Win32::InitCommonControlsEx(&icc);

	auto form = FormDesigner::Form{};
	if (lpCmdLine and lpCmdLine[0] != L'\0')
		form = FormDesigner::LoadFormFromFile(lpCmdLine);

	auto hwnd = Designer::CreateDesignSurface(hInstance, std::move(form));
	if (not hwnd)
		return 1;

	return Designer::RunDesignerLoop();
}
catch (const std::exception& ex)
{
	auto msg = std::string{ "Failed to start designer:\n" } + ex.what();
	auto wide = std::wstring(msg.begin(), msg.end());
	Win32::MessageBoxW(nullptr, wide.c_str(), L"Form Designer Error", Win32::Mb_IconError | Win32::Mb_Ok);
	return 1;
}
