#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static struct HotReloadCommonControlInit {
	HotReloadCommonControlInit()
	{
		Win32::INITCOMMONCONTROLSEX icc{ sizeof(Win32::INITCOMMONCONTROLSEX), Win32::Icc_StandardClasses };
		Win32::InitCommonControlsEx(&icc);
	}
} g_hotreloadCcInit;

static EventMap g_hotReloadEvents;

static auto MakeTestForm(const std::wstring& title = L"Test Form") -> Form
{
	auto form = Form{};
	form.title = title;
	form.width = 400;
	form.height = 300;
	form.visible = false;
	return form;
}

static auto WriteTempForm(const std::filesystem::path& path, const std::wstring& title,
	int width = 400, int height = 300) -> void
{
	auto narrowTitle = std::string{};
	for (auto ch : title)
		narrowTitle += static_cast<char>(ch);

	auto json = std::string(R"({"title":")") + narrowTitle
		+ R"(","width":)" + std::to_string(width)
		+ R"(,"height":)" + std::to_string(height)
		+ R"(,"controls":[]})";

	auto ofs = std::ofstream(path);
	ofs << json;
	ofs.close();
}

static auto WriteTempFormWithControl(const std::filesystem::path& path,
	const std::wstring& title, const std::string& controlText) -> void
{
	auto narrowTitle = std::string{};
	for (auto ch : title)
		narrowTitle += static_cast<char>(ch);

	auto json = std::string(R"({"title":")") + narrowTitle
		+ R"(","width":400,"height":300,"controls":[)"
		+ R"({"type":"Label","text":")" + controlText + R"(","rect":[10,10,100,20]}]})";

	auto ofs = std::ofstream(path);
	ofs << json;
	ofs.close();
}

// === Unit tests (no live windows) ===

TEST_CASE("EnableHotReload on null window data is safe", "[hotreload]")
{
	auto path = std::filesystem::temp_directory_path() / "hotreload_null_test.json";
	WriteTempForm(path, L"Test");

	// Should not crash on a bogus HWND (no window data).
	REQUIRE_NOTHROW(EnableHotReload(nullptr, path));

	std::filesystem::remove(path);
}

TEST_CASE("DisableHotReload on null window data is safe", "[hotreload]")
{
	REQUIRE_NOTHROW(DisableHotReload(nullptr));
}

// === Live window tests ===

TEST_CASE("EnableHotReload sets up timer on live window", "[hotreload]")
{
	auto path = std::filesystem::temp_directory_path() / "hotreload_enable_test.json";
	WriteTempForm(path, L"Original");

	auto form = LoadFormFromFile(path);
	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, g_hotReloadEvents);

	EnableHotReload(hwnd, path);

	// The window should still be valid.
	REQUIRE(Win32::IsWindow(hwnd));

	DisableHotReload(hwnd);
	Win32::DestroyWindow(hwnd);
	std::filesystem::remove(path);
}

TEST_CASE("DisableHotReload stops the timer", "[hotreload]")
{
	auto path = std::filesystem::temp_directory_path() / "hotreload_disable_test.json";
	WriteTempForm(path, L"Original");

	auto form = LoadFormFromFile(path);
	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, g_hotReloadEvents);

	EnableHotReload(hwnd, path);
	DisableHotReload(hwnd);

	// Should not crash and window should still be valid.
	REQUIRE(Win32::IsWindow(hwnd));

	Win32::DestroyWindow(hwnd);
	std::filesystem::remove(path);
}

TEST_CASE("Hot reload updates window title when file changes", "[hotreload]")
{
	auto path = std::filesystem::temp_directory_path() / "hotreload_title_test.json";
	WriteTempForm(path, L"Before");

	auto form = LoadFormFromFile(path);
	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, g_hotReloadEvents);

	EnableHotReload(hwnd, path, L"", 50); // fast polling for test

	// Verify initial title.
	wchar_t buf[256]{};
	Win32::GetWindowTextW(hwnd, buf, 256);
	REQUIRE(std::wstring(buf) == L"Before");

	// Ensure some time passes so the file timestamp differs.
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	// Modify the file.
	WriteTempForm(path, L"After");

	// Pump messages to process WM_TIMER.
	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
	while (std::chrono::steady_clock::now() < deadline)
	{
		auto msg = Win32::MSG{};
		while (Win32::PeekMessageW(&msg, nullptr, 0, 0, 1)) // PM_REMOVE = 1
		{
			Win32::TranslateMessage(&msg);
			Win32::DispatchMessageW(&msg);
		}
		Win32::GetWindowTextW(hwnd, buf, 256);
		if (std::wstring(buf) == L"After")
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	Win32::GetWindowTextW(hwnd, buf, 256);
	REQUIRE(std::wstring(buf) == L"After");

	DisableHotReload(hwnd);
	Win32::DestroyWindow(hwnd);
	std::filesystem::remove(path);
}

TEST_CASE("Hot reload recreates child controls", "[hotreload]")
{
	auto path = std::filesystem::temp_directory_path() / "hotreload_controls_test.json";
	WriteTempForm(path, L"Test");

	auto form = LoadFormFromFile(path);
	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, g_hotReloadEvents);

	// Initially no children.
	REQUIRE(Win32::GetWindow(hwnd, Win32::Gw_Child) == nullptr);

	EnableHotReload(hwnd, path, L"", 50);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	// Write form with a label.
	WriteTempFormWithControl(path, L"Test", "Hello");

	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
	auto hasChild = false;
	while (std::chrono::steady_clock::now() < deadline)
	{
		auto msg = Win32::MSG{};
		while (Win32::PeekMessageW(&msg, nullptr, 0, 0, 1))
		{
			Win32::TranslateMessage(&msg);
			Win32::DispatchMessageW(&msg);
		}
		if (Win32::GetWindow(hwnd, Win32::Gw_Child) != nullptr)
		{
			hasChild = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	REQUIRE(hasChild);

	DisableHotReload(hwnd);
	Win32::DestroyWindow(hwnd);
	std::filesystem::remove(path);
}

TEST_CASE("Hot reload survives parse error in file", "[hotreload]")
{
	auto path = std::filesystem::temp_directory_path() / "hotreload_error_test.json";
	WriteTempForm(path, L"Good");

	auto form = LoadFormFromFile(path);
	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, g_hotReloadEvents);

	EnableHotReload(hwnd, path, L"", 50);

	// Verify initial title.
	wchar_t buf[256]{};
	Win32::GetWindowTextW(hwnd, buf, 256);
	REQUIRE(std::wstring(buf) == L"Good");

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	// Write invalid JSON.
	{
		auto ofs = std::ofstream(path);
		ofs << "not valid json {{{";
		ofs.close();
	}

	// Pump messages — should not crash.
	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
	while (std::chrono::steady_clock::now() < deadline)
	{
		auto msg = Win32::MSG{};
		while (Win32::PeekMessageW(&msg, nullptr, 0, 0, 1))
		{
			Win32::TranslateMessage(&msg);
			Win32::DispatchMessageW(&msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Title should remain unchanged.
	Win32::GetWindowTextW(hwnd, buf, 256);
	REQUIRE(std::wstring(buf) == L"Good");

	DisableHotReload(hwnd);
	Win32::DestroyWindow(hwnd);
	std::filesystem::remove(path);
}

TEST_CASE("EnableHotReload can be called multiple times", "[hotreload]")
{
	auto path = std::filesystem::temp_directory_path() / "hotreload_multi_test.json";
	WriteTempForm(path, L"Test");

	auto form = LoadFormFromFile(path);
	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, g_hotReloadEvents);

	// Enable twice — should not leak timers.
	EnableHotReload(hwnd, path, L"", 100);
	EnableHotReload(hwnd, path, L"", 200);

	REQUIRE(Win32::IsWindow(hwnd));

	DisableHotReload(hwnd);
	Win32::DestroyWindow(hwnd);
	std::filesystem::remove(path);
}

TEST_CASE("Hot reload updates window size", "[hotreload]")
{
	auto path = std::filesystem::temp_directory_path() / "hotreload_size_test.json";
	WriteTempForm(path, L"Test", 400, 300);

	auto form = LoadFormFromFile(path);
	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, g_hotReloadEvents);

	EnableHotReload(hwnd, path, L"", 50);

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	// Write form with different size.
	WriteTempForm(path, L"Test", 600, 500);

	// Pump messages and wait for resize.
	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
	auto resized = false;
	while (std::chrono::steady_clock::now() < deadline)
	{
		auto msg = Win32::MSG{};
		while (Win32::PeekMessageW(&msg, nullptr, 0, 0, 1))
		{
			Win32::TranslateMessage(&msg);
			Win32::DispatchMessageW(&msg);
		}

		auto rc = Win32::RECT{};
		Win32::GetClientRect(hwnd, &rc);
		auto clientWidth = rc.right - rc.left;
		// Client width should have changed from original 400-based to 600-based.
		if (clientWidth > 450)
		{
			resized = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	REQUIRE(resized);

	DisableHotReload(hwnd);
	Win32::DestroyWindow(hwnd);
	std::filesystem::remove(path);
}
