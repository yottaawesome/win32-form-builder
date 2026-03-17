#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

// === Type traits and construction ===

TEST_CASE("ControlBase default constructs to null", "[controls]")
{
	auto ctrl = ControlBase{};
	REQUIRE(ctrl.Handle() == nullptr);
	REQUIRE(static_cast<bool>(ctrl) == false);
}

TEST_CASE("FormWindow default constructs to null", "[controls]")
{
	auto form = FormWindow{};
	REQUIRE(form.Handle() == nullptr);
	REQUIRE(static_cast<bool>(form) == false);
}

TEST_CASE("Button inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<Button, ControlBase>);
	auto btn = Button{};
	REQUIRE(btn.Handle() == nullptr);
}

TEST_CASE("TextBox inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<TextBox, ControlBase>);
}

TEST_CASE("CheckBox inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<CheckBox, ControlBase>);
}

TEST_CASE("RadioButton inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<RadioButton, ControlBase>);
}

TEST_CASE("Label inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<Label, ControlBase>);
}

TEST_CASE("ComboBox inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<ComboBox, ControlBase>);
}

TEST_CASE("ListBox inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<ListBox, ControlBase>);
}

TEST_CASE("ProgressBar inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<ProgressBar, ControlBase>);
}

TEST_CASE("TrackBar inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<TrackBar, ControlBase>);
}

TEST_CASE("UpDown inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<UpDown, ControlBase>);
}

TEST_CASE("DateTimePicker inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<DateTimePicker, ControlBase>);
}

TEST_CASE("RichEdit inherits from ControlBase", "[controls]")
{
	static_assert(std::derived_from<RichEdit, ControlBase>);
}

// === FormWindow::Get<T> template constraint ===

TEST_CASE("FormWindow Get<T> requires ControlBase-derived type", "[controls]")
{
	static_assert(requires(FormWindow fw) { fw.Get<Button>(1); });
	static_assert(requires(FormWindow fw) { fw.Get<TextBox>(1); });
	static_assert(requires(FormWindow fw) { fw.Get<CheckBox>(1); });
	static_assert(requires(FormWindow fw) { fw.Get<ComboBox>(1); });
	static_assert(requires(FormWindow fw) { fw.Get<ListBox>(1); });
	static_assert(requires(FormWindow fw) { fw.Get<ProgressBar>(1); });
	static_assert(requires(FormWindow fw) { fw.Get<TrackBar>(1); });
	static_assert(requires(FormWindow fw) { fw.Get<UpDown>(1); });
	static_assert(requires(FormWindow fw) { fw.Get<RichEdit>(1); });
}

// === Live control tests using LoadForm ===

// Ensure common controls are initialized once.
static struct CommonControlInit {
	CommonControlInit()
	{
		Win32::INITCOMMONCONTROLSEX icc{ sizeof(Win32::INITCOMMONCONTROLSEX), Win32::Icc_StandardClasses };
		Win32::InitCommonControlsEx(&icc);
	}
} g_ccInit;

// Shared EventMap — LoadForm stores a pointer, so it must outlive the window.
static EventMap g_events;

// Helper: creates a form with specified controls, loads it, and returns the FormWindow.
// The form is created hidden so tests can run headlessly.
static auto MakeTestForm(std::vector<Control> controls) -> FormWindow
{
	auto form = Form{};
	form.title = L"Test";
	form.width = 400;
	form.height = 300;
	form.visible = false;
	form.controls = std::move(controls);

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto hwnd = LoadForm(form, hInstance, g_events);
	return FormWindow{hwnd};
}

TEST_CASE("FormWindow GetTitle returns form title", "[controls]")
{
	auto window = MakeTestForm({});
	REQUIRE(window.GetTitle() == L"Test");
	window.Close();
}

TEST_CASE("FormWindow SetTitle changes title", "[controls]")
{
	auto window = MakeTestForm({});
	window.SetTitle(L"New Title");
	REQUIRE(window.GetTitle() == L"New Title");
	window.Close();
}

TEST_CASE("FormWindow Show/Hide/IsVisible", "[controls]")
{
	auto window = MakeTestForm({});
	REQUIRE(window.IsVisible() == false);
	window.Show();
	REQUIRE(window.IsVisible() == true);
	window.Hide();
	REQUIRE(window.IsVisible() == false);
	window.Close();
}

TEST_CASE("FormWindow Enable/Disable/IsEnabled", "[controls]")
{
	auto window = MakeTestForm({});
	REQUIRE(window.IsEnabled() == true);
	window.Disable();
	REQUIRE(window.IsEnabled() == false);
	window.Enable();
	REQUIRE(window.IsEnabled() == true);
	window.Close();
}

TEST_CASE("TextBox GetText/SetText via FormWindow", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::TextBox;
	ctrl.text = L"Hello";
	ctrl.id = 101;
	ctrl.rect = { 10, 10, 150, 25 };

	auto window = MakeTestForm({ ctrl });
	auto textBox = window.GetTextBox(101);
	REQUIRE(static_cast<bool>(textBox));
	REQUIRE(textBox.GetText() == L"Hello");
	textBox.SetText(L"World");
	REQUIRE(textBox.GetText() == L"World");
	window.Close();
}

TEST_CASE("TextBox GetTextLength", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::TextBox;
	ctrl.text = L"ABCDE";
	ctrl.id = 101;
	ctrl.rect = { 10, 10, 150, 25 };

	auto window = MakeTestForm({ ctrl });
	auto textBox = window.GetTextBox(101);
	REQUIRE(textBox.GetTextLength() == 5);
	window.Close();
}

TEST_CASE("CheckBox IsChecked/SetChecked/Toggle", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::CheckBox;
	ctrl.text = L"Check me";
	ctrl.id = 102;
	ctrl.rect = { 10, 40, 150, 25 };

	auto window = MakeTestForm({ ctrl });
	auto chk = window.GetCheckBox(102);
	REQUIRE(chk.IsChecked() == false);
	chk.SetChecked(true);
	REQUIRE(chk.IsChecked() == true);
	chk.Toggle();
	REQUIRE(chk.IsChecked() == false);
	window.Close();
}

TEST_CASE("RadioButton IsSelected/SetSelected", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::RadioButton;
	ctrl.text = L"Option A";
	ctrl.id = 103;
	ctrl.rect = { 10, 70, 150, 25 };

	auto window = MakeTestForm({ ctrl });
	auto radio = window.GetRadioButton(103);
	REQUIRE(radio.IsSelected() == false);
	radio.SetSelected(true);
	REQUIRE(radio.IsSelected() == true);
	window.Close();
}

TEST_CASE("Label GetText/SetText", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::Label;
	ctrl.text = L"Status:";
	ctrl.id = 104;
	ctrl.rect = { 10, 100, 150, 20 };

	auto window = MakeTestForm({ ctrl });
	auto lbl = window.GetLabel(104);
	REQUIRE(lbl.GetText() == L"Status:");
	lbl.SetText(L"Done!");
	REQUIRE(lbl.GetText() == L"Done!");
	window.Close();
}

TEST_CASE("Button GetText/Enable/Disable", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::Button;
	ctrl.text = L"OK";
	ctrl.id = 105;
	ctrl.rect = { 10, 130, 80, 30 };

	auto window = MakeTestForm({ ctrl });
	auto btn = window.GetButton(105);
	REQUIRE(btn.GetText() == L"OK");
	REQUIRE(btn.IsEnabled() == true);
	btn.Disable();
	REQUIRE(btn.IsEnabled() == false);
	btn.Enable();
	REQUIRE(btn.IsEnabled() == true);
	window.Close();
}

TEST_CASE("ComboBox AddItem/GetCount/GetSelectedIndex/SetSelectedIndex", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::ComboBox;
	ctrl.id = 106;
	ctrl.rect = { 10, 170, 150, 100 };

	auto window = MakeTestForm({ ctrl });
	auto combo = window.GetComboBox(106);
	REQUIRE(combo.GetCount() == 0);

	combo.AddItem(L"Alpha");
	combo.AddItem(L"Beta");
	combo.AddItem(L"Gamma");
	REQUIRE(combo.GetCount() == 3);
	REQUIRE(combo.GetItemText(0) == L"Alpha");
	REQUIRE(combo.GetItemText(1) == L"Beta");

	combo.SetSelectedIndex(1);
	REQUIRE(combo.GetSelectedIndex() == 1);

	combo.RemoveItem(0);
	REQUIRE(combo.GetCount() == 2);
	REQUIRE(combo.GetItemText(0) == L"Beta");

	combo.Clear();
	REQUIRE(combo.GetCount() == 0);
	window.Close();
}

TEST_CASE("ComboBox InsertItem", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::ComboBox;
	ctrl.id = 106;
	ctrl.rect = { 10, 10, 150, 100 };

	auto window = MakeTestForm({ ctrl });
	auto combo = window.GetComboBox(106);
	combo.AddItem(L"First");
	combo.AddItem(L"Third");
	combo.InsertItem(1, L"Second");
	REQUIRE(combo.GetCount() == 3);
	REQUIRE(combo.GetItemText(1) == L"Second");
	window.Close();
}

TEST_CASE("ListBox AddItem/GetCount/GetSelectedIndex/SetSelectedIndex", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::ListBox;
	ctrl.id = 107;
	ctrl.rect = { 10, 10, 150, 80 };

	auto window = MakeTestForm({ ctrl });
	auto lb = window.GetListBox(107);
	REQUIRE(lb.GetCount() == 0);

	lb.AddItem(L"Item 1");
	lb.AddItem(L"Item 2");
	REQUIRE(lb.GetCount() == 2);
	REQUIRE(lb.GetItemText(0) == L"Item 1");

	lb.SetSelectedIndex(0);
	REQUIRE(lb.GetSelectedIndex() == 0);

	lb.Clear();
	REQUIRE(lb.GetCount() == 0);
	window.Close();
}

TEST_CASE("ProgressBar SetValue/GetValue/SetRange", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::ProgressBar;
	ctrl.id = 108;
	ctrl.rect = { 10, 10, 200, 25 };

	auto window = MakeTestForm({ ctrl });
	auto pb = window.GetProgressBar(108);
	pb.SetRange(0, 200);
	pb.SetValue(75);
	REQUIRE(pb.GetValue() == 75);
	window.Close();
}

TEST_CASE("ProgressBar Step", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::ProgressBar;
	ctrl.id = 108;
	ctrl.rect = { 10, 10, 200, 25 };

	auto window = MakeTestForm({ ctrl });
	auto pb = window.GetProgressBar(108);
	pb.SetRange(0, 100);
	pb.SetValue(0);
	pb.SetStepSize(10);
	pb.Step();
	REQUIRE(pb.GetValue() == 10);
	pb.Step();
	REQUIRE(pb.GetValue() == 20);
	window.Close();
}

TEST_CASE("TrackBar SetValue/GetValue/SetRange", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::TrackBar;
	ctrl.id = 109;
	ctrl.rect = { 10, 10, 200, 40 };

	auto window = MakeTestForm({ ctrl });
	auto tb = window.GetTrackBar(109);
	tb.SetRange(0, 50);
	tb.SetValue(25);
	REQUIRE(tb.GetValue() == 25);
	window.Close();
}

TEST_CASE("UpDown SetValue/GetValue/SetRange", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::UpDown;
	ctrl.id = 110;
	ctrl.rect = { 10, 10, 50, 25 };

	auto window = MakeTestForm({ ctrl });
	auto ud = window.GetUpDown(110);
	ud.SetRange(0, 999);
	ud.SetValue(42);
	REQUIRE(ud.GetValue() == 42);
	window.Close();
}

TEST_CASE("Control Show/Hide/IsVisible", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::Button;
	ctrl.text = L"Test";
	ctrl.id = 111;
	ctrl.rect = { 10, 10, 80, 30 };

	auto window = MakeTestForm({ ctrl });
	window.Show(); // Parent must be visible for child IsVisible to return true.
	auto btn = window.GetButton(111);
	REQUIRE(btn.IsVisible() == true);
	btn.Hide();
	REQUIRE(btn.IsVisible() == false);
	btn.Show();
	REQUIRE(btn.IsVisible() == true);
	window.Close();
}

TEST_CASE("Control Enable/Disable/IsEnabled", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::TextBox;
	ctrl.text = L"";
	ctrl.id = 112;
	ctrl.rect = { 10, 10, 150, 25 };

	auto window = MakeTestForm({ ctrl });
	auto tb = window.GetTextBox(112);
	REQUIRE(tb.IsEnabled() == true);
	tb.Disable();
	REQUIRE(tb.IsEnabled() == false);
	tb.Enable();
	REQUIRE(tb.IsEnabled() == true);
	window.Close();
}

TEST_CASE("FormWindow Get<T> returns typed wrapper", "[controls]")
{
	auto ctrl = Control{};
	ctrl.type = ControlType::Button;
	ctrl.text = L"Test";
	ctrl.id = 113;
	ctrl.rect = { 10, 10, 80, 30 };

	auto window = MakeTestForm({ ctrl });
	auto btn = window.Get<Button>(113);
	REQUIRE(btn.GetText() == L"Test");
	window.Close();
}
