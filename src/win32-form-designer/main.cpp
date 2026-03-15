#pragma comment(lib, "Comctl32.lib")

import std;
import formbuilder;

auto __stdcall wWinMain(Win32::HINSTANCE hInstance, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
try
{
	auto icc = Win32::INITCOMMONCONTROLSEX{
		.dwSize = sizeof(Win32::INITCOMMONCONTROLSEX),
		.dwICC = Win32::Icc_StandardClasses,
	};
	Win32::InitCommonControlsEx(&icc);

	auto form = FormDesigner::Form{
		.title = L"Form Designer",
		.width = 800,
		.height = 600,
	};

	auto events = FormDesigner::EventMap{};
	auto hwnd = FormDesigner::LoadForm(form, hInstance, events);
	if (not hwnd)
		return 1;

	return FormDesigner::RunMessageLoop();
}
catch (const std::exception& ex)
{
	auto msg = std::string{ "Failed to start designer:\n" } + ex.what();
	auto wide = std::wstring(msg.begin(), msg.end());
	Win32::MessageBoxW(nullptr, wide.c_str(), L"Form Designer Error", Win32::Mb_IconError | Win32::Mb_Ok);
	return 1;
}
