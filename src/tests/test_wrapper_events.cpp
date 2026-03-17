#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

static struct WrapperEventCommonControlInit {
	WrapperEventCommonControlInit()
	{
		Win32::INITCOMMONCONTROLSEX icc{ sizeof(Win32::INITCOMMONCONTROLSEX), Win32::Icc_StandardClasses };
		Win32::InitCommonControlsEx(&icc);
	}
} g_wrapperEventCcInit;

// Helper: create a form with specific controls, load it, return HWND.
static auto MakeFormWithButton(EventMap& events, int buttonId = 100) -> Win32::HWND
{
	auto form = Form{};
	form.title = L"Wrapper Event Test";
	form.width = 300;
	form.height = 200;
	form.visible = false;

	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"Test";
	btn.id = buttonId;
	btn.rect = { 10, 10, 80, 30 };
	form.controls = { btn };

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	return LoadForm(form, hInstance, events);
}

static auto MakeFormWithCheckBox(EventMap& events, int checkId = 200) -> Win32::HWND
{
	auto form = Form{};
	form.title = L"CheckBox Event Test";
	form.width = 300;
	form.height = 200;
	form.visible = false;

	auto chk = Control{};
	chk.type = ControlType::CheckBox;
	chk.text = L"Check";
	chk.id = checkId;
	chk.rect = { 10, 10, 100, 20 };
	form.controls = { chk };

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	return LoadForm(form, hInstance, events);
}

static auto MakeFormWithTextBox(EventMap& events, int textId = 300) -> Win32::HWND
{
	auto form = Form{};
	form.title = L"TextBox Event Test";
	form.width = 300;
	form.height = 200;
	form.visible = false;

	auto txt = Control{};
	txt.type = ControlType::TextBox;
	txt.text = L"";
	txt.id = textId;
	txt.rect = { 10, 10, 200, 24 };
	form.controls = { txt };

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	return LoadForm(form, hInstance, events);
}

// === FormWindow retrieves EventMap ===

TEST_CASE("FormWindow retrieves EventMap from loaded form", "[wrapper-events]")
{
	auto events = EventMap{};
	auto hwnd = MakeFormWithButton(events);
	auto window = FormWindow{hwnd};

	// Get<T>() should produce wrappers that can bind events.
	auto button = window.GetButton(100);
	REQUIRE(button.Handle() != nullptr);

	// OnClick should not throw — EventMap is available.
	auto clicked = false;
	REQUIRE_NOTHROW(button.OnClick([&](const ClickEvent&) { clicked = true; }));

	Win32::DestroyWindow(hwnd);
}

TEST_CASE("FormWindow convenience accessors pass EventMap", "[wrapper-events]")
{
	auto events = EventMap{};
	auto hwnd = MakeFormWithCheckBox(events);
	auto window = FormWindow{hwnd};

	auto checkbox = window.GetCheckBox(200);
	REQUIRE(checkbox.Handle() != nullptr);

	auto checked = false;
	REQUIRE_NOTHROW(checkbox.OnCheck([&](const CheckEvent& e) { checked = e.checked; }));

	Win32::DestroyWindow(hwnd);
}

// === RequireEvents throws without EventMap ===

TEST_CASE("Button::OnClick throws without EventMap", "[wrapper-events]")
{
	auto events = EventMap{};
	auto hwnd = MakeFormWithButton(events);

	// Construct wrapper directly (no EventMap).
	auto buttonHwnd = Win32::GetDlgItem(hwnd, 100);
	auto button = Button{buttonHwnd};

	REQUIRE_THROWS_AS(button.OnClick([](const ClickEvent&) {}), FormException);

	Win32::DestroyWindow(hwnd);
}

TEST_CASE("TextBox::OnChange throws without EventMap", "[wrapper-events]")
{
	auto events = EventMap{};
	auto hwnd = MakeFormWithTextBox(events);

	auto textHwnd = Win32::GetDlgItem(hwnd, 300);
	auto textBox = TextBox{textHwnd};

	REQUIRE_THROWS_AS(textBox.OnChange([](const ChangeEvent&) {}), FormException);

	Win32::DestroyWindow(hwnd);
}

// === Event binding through wrappers dispatches correctly ===

TEST_CASE("Button::OnClick registers handler that fires on BN_CLICKED", "[wrapper-events]")
{
	auto events = EventMap{};
	auto hwnd = MakeFormWithButton(events, 101);
	auto window = FormWindow{hwnd};

	auto clickedId = 0;
	window.GetButton(101).OnClick([&](const ClickEvent& e) {
		clickedId = e.controlId;
	});

	// Simulate BN_CLICKED via WM_COMMAND.
	auto buttonHwnd = Win32::GetDlgItem(hwnd, 101);
	Win32::SendMessageW(hwnd, Win32::Messages::Command,
		Win32::MakeWParam(101, Win32::Notifications::ButtonClicked),
		reinterpret_cast<Win32::LPARAM>(buttonHwnd));

	REQUIRE(clickedId == 101);

	Win32::DestroyWindow(hwnd);
}

TEST_CASE("CheckBox::OnCheck registers handler that fires on BN_CLICKED", "[wrapper-events]")
{
	auto events = EventMap{};
	auto hwnd = MakeFormWithCheckBox(events, 201);
	auto window = FormWindow{hwnd};

	auto wasChecked = false;
	window.GetCheckBox(201).OnCheck([&](const CheckEvent& e) {
		wasChecked = e.checked;
	});

	// Set check state first, then simulate click.
	auto chkHwnd = Win32::GetDlgItem(hwnd, 201);
	Win32::SendMessageW(chkHwnd, Win32::Button::SetCheck, Win32::Button::Checked, 0);
	Win32::SendMessageW(hwnd, Win32::Messages::Command,
		Win32::MakeWParam(201, Win32::Notifications::ButtonClicked),
		reinterpret_cast<Win32::LPARAM>(chkHwnd));

	REQUIRE(wasChecked == true);

	Win32::DestroyWindow(hwnd);
}

// === FormWindow-level event binding ===

TEST_CASE("FormWindow::OnClick registers handler by control ID", "[wrapper-events]")
{
	auto events = EventMap{};
	auto hwnd = MakeFormWithButton(events, 102);
	auto window = FormWindow{hwnd};

	auto clickedId = 0;
	window.OnClick(102, [&](const ClickEvent& e) {
		clickedId = e.controlId;
	});

	auto buttonHwnd = Win32::GetDlgItem(hwnd, 102);
	Win32::SendMessageW(hwnd, Win32::Messages::Command,
		Win32::MakeWParam(102, Win32::Notifications::ButtonClicked),
		reinterpret_cast<Win32::LPARAM>(buttonHwnd));

	REQUIRE(clickedId == 102);

	Win32::DestroyWindow(hwnd);
}

// === Backward compatibility ===

TEST_CASE("EventMap-first approach still works", "[wrapper-events]")
{
	auto events = EventMap{};
	auto clickedId = 0;

	// Register via EventMap BEFORE LoadForm (old approach).
	events.onClick(103, [&](const ClickEvent& e) {
		clickedId = e.controlId;
	});

	auto form = Form{};
	form.title = L"Compat Test";
	form.width = 300;
	form.height = 200;
	form.visible = false;

	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"Compat";
	btn.id = 103;
	btn.rect = { 10, 10, 80, 30 };
	form.controls = { btn };

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, events);

	auto buttonHwnd = Win32::GetDlgItem(hwnd, 103);
	Win32::SendMessageW(hwnd, Win32::Messages::Command,
		Win32::MakeWParam(103, Win32::Notifications::ButtonClicked),
		reinterpret_cast<Win32::LPARAM>(buttonHwnd));

	REQUIRE(clickedId == 103);

	Win32::DestroyWindow(hwnd);
}

// === Mixed approach: EventMap + wrapper binding ===

TEST_CASE("EventMap and wrapper binding can coexist", "[wrapper-events]")
{
	auto events = EventMap{};

	// Register one handler via EventMap before LoadForm.
	auto clickedViaMap = false;
	events.onClick(104, [&](const ClickEvent&) { clickedViaMap = true; });

	auto form = Form{};
	form.title = L"Mixed Test";
	form.width = 300;
	form.height = 200;
	form.visible = false;

	auto btn1 = Control{};
	btn1.type = ControlType::Button;
	btn1.text = L"MapBtn";
	btn1.id = 104;
	btn1.rect = { 10, 10, 80, 30 };

	auto btn2 = Control{};
	btn2.type = ControlType::Button;
	btn2.text = L"WrapBtn";
	btn2.id = 105;
	btn2.rect = { 10, 50, 80, 30 };

	form.controls = { btn1, btn2 };

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, events);
	auto window = FormWindow{hwnd};

	// Register second handler via wrapper after LoadForm.
	auto clickedViaWrapper = false;
	window.GetButton(105).OnClick([&](const ClickEvent&) { clickedViaWrapper = true; });

	// Fire both.
	auto btn1Hwnd = Win32::GetDlgItem(hwnd, 104);
	Win32::SendMessageW(hwnd, Win32::Messages::Command,
		Win32::MakeWParam(104, Win32::Notifications::ButtonClicked),
		reinterpret_cast<Win32::LPARAM>(btn1Hwnd));

	auto btn2Hwnd = Win32::GetDlgItem(hwnd, 105);
	Win32::SendMessageW(hwnd, Win32::Messages::Command,
		Win32::MakeWParam(105, Win32::Notifications::ButtonClicked),
		reinterpret_cast<Win32::LPARAM>(btn2Hwnd));

	REQUIRE(clickedViaMap);
	REQUIRE(clickedViaWrapper);

	Win32::DestroyWindow(hwnd);
}

// === Wrapper can override EventMap handler ===

TEST_CASE("Wrapper binding overrides previous EventMap handler", "[wrapper-events]")
{
	auto events = EventMap{};

	auto originalFired = false;
	events.onClick(106, [&](const ClickEvent&) { originalFired = true; });

	auto form = Form{};
	form.title = L"Override Test";
	form.width = 300;
	form.height = 200;
	form.visible = false;

	auto btn = Control{};
	btn.type = ControlType::Button;
	btn.text = L"Override";
	btn.id = 106;
	btn.rect = { 10, 10, 80, 30 };
	form.controls = { btn };

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, events);
	auto window = FormWindow{hwnd};

	auto overrideFired = false;
	window.GetButton(106).OnClick([&](const ClickEvent&) { overrideFired = true; });

	auto btnHwnd = Win32::GetDlgItem(hwnd, 106);
	Win32::SendMessageW(hwnd, Win32::Messages::Command,
		Win32::MakeWParam(106, Win32::Notifications::ButtonClicked),
		reinterpret_cast<Win32::LPARAM>(btnHwnd));

	REQUIRE_FALSE(originalFired);
	REQUIRE(overrideFired);

	Win32::DestroyWindow(hwnd);
}

// === ControlBase constructor with EventMap ===

TEST_CASE("ControlBase two-arg constructor stores EventMap", "[wrapper-events]")
{
	auto events = EventMap{};
	auto hwnd = MakeFormWithButton(events, 107);

	auto buttonHwnd = Win32::GetDlgItem(hwnd, 107);
	auto button = Button{buttonHwnd, &events};

	// Should not throw since EventMap is provided.
	REQUIRE_NOTHROW(button.OnClick([](const ClickEvent&) {}));

	Win32::DestroyWindow(hwnd);
}
