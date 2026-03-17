export module designer:templates;
import std;
import formbuilder;

using FormDesigner::Form;
using FormDesigner::Control;
using FormDesigner::ControlType;
using FormDesigner::TextAlign;
using FormDesigner::Rect;

namespace
{
	auto MakeControl(ControlType type, const std::wstring& text, Rect rect, int id) -> Control
	{
		auto c = Control{};
		c.type = type;
		c.text = text;
		c.rect = rect;
		c.id = id;
		return c;
	}
}

export namespace Designer
{
	auto TemplateLoginDialog() -> Form
	{
		auto form = Form{};
		form.title = L"Login";
		form.width = 340;
		form.height = 220;

		auto lbUser = MakeControl(ControlType::Label, L"Username:", { 20, 20, 80, 18 }, 101);
		auto tbUser = MakeControl(ControlType::TextBox, L"", { 110, 18, 200, 22 }, 102);
		tbUser.bindField = "username";

		auto lbPass = MakeControl(ControlType::Label, L"Password:", { 20, 55, 80, 18 }, 103);
		auto tbPass = MakeControl(ControlType::TextBox, L"", { 110, 53, 200, 22 }, 104);
		tbPass.bindField = "password";

		auto chkRemember = MakeControl(ControlType::CheckBox, L"Remember me", { 110, 85, 120, 20 }, 105);
		chkRemember.bindField = "rememberMe";

		auto btnLogin = MakeControl(ControlType::Button, L"Login", { 130, 130, 80, 28 }, 106);
		btnLogin.onClick = "OnLogin";

		auto btnCancel = MakeControl(ControlType::Button, L"Cancel", { 220, 130, 80, 28 }, 107);
		btnCancel.onClick = "OnCancel";

		form.controls = { lbUser, tbUser, lbPass, tbPass, chkRemember, btnLogin, btnCancel };
		form.bindStruct = "LoginCredentials";
		return form;
	}

	auto TemplateSettingsDialog() -> Form
	{
		auto form = Form{};
		form.title = L"Settings";
		form.width = 450;
		form.height = 380;

		auto tabs = MakeControl(ControlType::TabControl, L"General\tAdvanced", { 10, 10, 420, 280 }, 101);

		// General tab controls.
		auto chk1 = MakeControl(ControlType::CheckBox, L"Enable notifications", { 30, 50, 160, 20 }, 102);
		auto chk2 = MakeControl(ControlType::CheckBox, L"Start minimized", { 30, 75, 160, 20 }, 103);
		auto chk3 = MakeControl(ControlType::CheckBox, L"Check for updates", { 30, 100, 160, 20 }, 104);

		auto lbLang = MakeControl(ControlType::Label, L"Language:", { 30, 135, 70, 18 }, 105);
		auto cbLang = MakeControl(ControlType::ComboBox, L"", { 110, 132, 150, 200 }, 106);
		cbLang.items = { L"English", L"Spanish", L"French", L"German", L"Japanese" };
		cbLang.selectedIndex = 0;

		// Advanced tab controls.
		auto lbLog = MakeControl(ControlType::Label, L"Log level:", { 30, 175, 70, 18 }, 107);
		auto cbLog = MakeControl(ControlType::ComboBox, L"", { 110, 172, 150, 200 }, 108);
		cbLog.items = { L"Error", L"Warning", L"Info", L"Debug" };
		cbLog.selectedIndex = 2;

		auto lbPort = MakeControl(ControlType::Label, L"Port:", { 30, 205, 70, 18 }, 109);
		auto tbPort = MakeControl(ControlType::TextBox, L"8080", { 110, 203, 80, 22 }, 110);

		auto btnOK = MakeControl(ControlType::Button, L"OK", { 240, 310, 80, 28 }, 111);
		btnOK.onClick = "OnOK";
		auto btnCancel = MakeControl(ControlType::Button, L"Cancel", { 330, 310, 80, 28 }, 112);
		btnCancel.onClick = "OnCancel";
		auto btnApply = MakeControl(ControlType::Button, L"Apply", { 150, 310, 80, 28 }, 113);
		btnApply.onClick = "OnApply";

		form.controls = { tabs, chk1, chk2, chk3, lbLang, cbLang, lbLog, cbLog, lbPort, tbPort, btnOK, btnCancel, btnApply };
		return form;
	}

	auto TemplateDataEntryForm() -> Form
	{
		auto form = Form{};
		form.title = L"Data Entry";
		form.width = 480;
		form.height = 380;

		int y = 15;
		constexpr int lx = 20, lw = 90, ex = 120, ew = 220, rh = 35;
		int id = 101;

		auto lbFirst = MakeControl(ControlType::Label, L"First Name:", { lx, y + 2, lw, 18 }, id++);
		auto tbFirst = MakeControl(ControlType::TextBox, L"", { ex, y, ew, 22 }, id++);
		tbFirst.bindField = "firstName";
		tbFirst.validation.required = true;
		y += rh;

		auto lbLast = MakeControl(ControlType::Label, L"Last Name:", { lx, y + 2, lw, 18 }, id++);
		auto tbLast = MakeControl(ControlType::TextBox, L"", { ex, y, ew, 22 }, id++);
		tbLast.bindField = "lastName";
		tbLast.validation.required = true;
		y += rh;

		auto lbEmail = MakeControl(ControlType::Label, L"Email:", { lx, y + 2, lw, 18 }, id++);
		auto tbEmail = MakeControl(ControlType::TextBox, L"", { ex, y, ew, 22 }, id++);
		tbEmail.bindField = "email";
		tbEmail.validation.pattern = "^[\\w.-]+@[\\w.-]+\\.\\w+$";
		y += rh;

		auto lbDob = MakeControl(ControlType::Label, L"Date of Birth:", { lx, y + 2, lw, 18 }, id++);
		auto dtDob = MakeControl(ControlType::DateTimePicker, L"", { ex, y, ew, 22 }, id++);
		y += rh;

		auto lbCategory = MakeControl(ControlType::Label, L"Category:", { lx, y + 2, lw, 18 }, id++);
		auto cbCategory = MakeControl(ControlType::ComboBox, L"", { ex, y, ew, 200 }, id++);
		cbCategory.items = { L"Customer", L"Employee", L"Vendor", L"Partner" };
		cbCategory.selectedIndex = 0;
		cbCategory.bindField = "categoryIndex";
		y += rh;

		auto lbNotes = MakeControl(ControlType::Label, L"Notes:", { lx, y + 2, lw, 18 }, id++);
		auto tbNotes = MakeControl(ControlType::RichEdit, L"", { ex, y, ew, 80 }, id++);
		tbNotes.bindField = "notes";
		y += 95;

		auto sep = MakeControl(ControlType::Separator, L"", { 10, y, 450, 2 }, id++);
		y += 15;

		auto btnSave = MakeControl(ControlType::Button, L"Save", { 250, y, 80, 28 }, id++);
		btnSave.onClick = "OnSave";
		auto btnCancel = MakeControl(ControlType::Button, L"Cancel", { 340, y, 80, 28 }, id++);
		btnCancel.onClick = "OnCancel";

		form.controls = { lbFirst, tbFirst, lbLast, tbLast, lbEmail, tbEmail,
			lbDob, dtDob, lbCategory, cbCategory, lbNotes, tbNotes, sep, btnSave, btnCancel };
		form.bindStruct = "PersonRecord";
		return form;
	}

	auto TemplateAboutDialog() -> Form
	{
		auto form = Form{};
		form.title = L"About";
		form.width = 340;
		form.height = 220;

		auto pic = MakeControl(ControlType::Picture, L"", { 20, 20, 64, 64 }, 101);

		auto lbName = MakeControl(ControlType::Label, L"Application Name", { 100, 25, 210, 22 }, 102);
		lbName.font.size = 14;
		lbName.font.bold = true;

		auto lbVer = MakeControl(ControlType::Label, L"Version 1.0.0", { 100, 52, 210, 18 }, 103);

		auto lbDesc = MakeControl(ControlType::Label, L"A brief description of the application goes here.",
			{ 20, 100, 290, 36 }, 104);

		auto link = MakeControl(ControlType::Link, L"<a>Visit website</a>", { 20, 140, 150, 18 }, 105);

		auto btnOK = MakeControl(ControlType::Button, L"OK", { 230, 150, 80, 28 }, 106);
		btnOK.onClick = "OnOK";

		form.controls = { pic, lbName, lbVer, lbDesc, link, btnOK };
		return form;
	}

	auto TemplateSearchForm() -> Form
	{
		auto form = Form{};
		form.title = L"Search";
		form.width = 560;
		form.height = 420;

		auto lbSearch = MakeControl(ControlType::Label, L"Search:", { 10, 12, 50, 18 }, 101);
		auto tbSearch = MakeControl(ControlType::TextBox, L"", { 65, 10, 360, 22 }, 102);
		tbSearch.bindField = "query";

		auto btnSearch = MakeControl(ControlType::Button, L"Search", { 435, 10, 80, 24 }, 103);
		btnSearch.onClick = "OnSearch";

		auto btnClear = MakeControl(ControlType::Button, L"Clear", { 435, 40, 80, 24 }, 104);
		btnClear.onClick = "OnClear";

		auto lv = MakeControl(ControlType::ListView, L"", { 10, 45, 415, 310 }, 105);

		auto lbStatus = MakeControl(ControlType::Label, L"Ready", { 10, 365, 400, 18 }, 106);
		lbStatus.bindField = "statusText";

		auto pb = MakeControl(ControlType::ProgressBar, L"", { 435, 365, 80, 18 }, 107);
		pb.visible = false;

		form.controls = { lbSearch, tbSearch, btnSearch, btnClear, lv, lbStatus, pb };
		form.bindStruct = "SearchState";
		return form;
	}
}
