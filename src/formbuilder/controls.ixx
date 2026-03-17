export module formbuilder:controls;
import std;
import :win32;
import :events;
import :errors;

export namespace FormDesigner
{
	// Non-owning wrapper around any Win32 control HWND.
	// When constructed with an EventMap*, supports event binding (OnClick, etc.).
	struct ControlBase
	{
		ControlBase() = default;
		explicit ControlBase(Win32::HWND h) : hwnd_{h} {}
		ControlBase(Win32::HWND h, EventMap* e) : hwnd_{h}, events_{e} {}

		auto Handle() const noexcept -> Win32::HWND { return hwnd_; }
		explicit operator bool() const noexcept { return hwnd_ != nullptr; }

		auto GetText() const -> std::wstring
		{
			auto len = Win32::GetWindowTextLengthW(hwnd_);
			if (len <= 0) return {};
			auto buf = std::wstring(static_cast<size_t>(len) + 1, L'\0');
			Win32::GetWindowTextW(hwnd_, buf.data(), len + 1);
			buf.resize(static_cast<size_t>(len));
			return buf;
		}

		void SetText(std::wstring_view text)
		{
			Win32::SetWindowTextW(hwnd_, std::wstring{text}.c_str());
		}

		void Show()    { Win32::ShowWindow(hwnd_, Win32::Sw_Show); }
		void Hide()    { Win32::ShowWindow(hwnd_, Win32::Sw_Hide); }
		void Enable()  { Win32::EnableWindow(hwnd_, true); }
		void Disable() { Win32::EnableWindow(hwnd_, false); }
		void Focus()   { Win32::SetFocus(hwnd_); }

		auto IsVisible() const -> bool { return Win32::IsWindowVisible(hwnd_) != 0; }
		auto IsEnabled() const -> bool { return Win32::IsWindowEnabled(hwnd_) != 0; }

	protected:
		Win32::HWND hwnd_ = nullptr;
		EventMap* events_ = nullptr;

		auto GetControlId() const -> int
		{
			return Win32::GetDlgCtrlID(hwnd_);
		}

		auto RequireEvents() -> EventMap&
		{
			if (!events_)
				throw FormException(FormErrorCode::InvalidField,
					"Event binding requires a wrapper obtained from FormWindow");
			return *events_;
		}
	};

	// Button control wrapper.
	struct Button : ControlBase
	{
		using ControlBase::ControlBase;

		void OnClick(std::function<void(const ClickEvent&)> handler)
		{
			RequireEvents().onClick(GetControlId(), std::move(handler));
		}

		void OnDoubleClick(std::function<void(const DoubleClickEvent&)> handler)
		{
			RequireEvents().onDoubleClick(GetControlId(), std::move(handler));
		}

		void OnFocus(std::function<void(const FocusEvent&)> handler)
		{
			RequireEvents().onFocus(GetControlId(), std::move(handler));
		}

		void OnBlur(std::function<void(const BlurEvent&)> handler)
		{
			RequireEvents().onBlur(GetControlId(), std::move(handler));
		}
	};

	// Label (STATIC) control wrapper.
	struct Label : ControlBase
	{
		using ControlBase::ControlBase;
	};

	// TextBox (EDIT) control wrapper.
	struct TextBox : ControlBase
	{
		using ControlBase::ControlBase;

		void SetReadOnly(bool readOnly = true)
		{
			Win32::SendMessageW(hwnd_, Win32::EditControl::SetReadOnly,
				readOnly ? 1 : 0, 0);
		}

		void SelectAll()
		{
			Win32::SendMessageW(hwnd_, Win32::EditControl::SetSel, 0, -1);
		}

		void SetSelection(int start, int end)
		{
			Win32::SendMessageW(hwnd_, Win32::EditControl::SetSel, start, end);
		}

		auto GetTextLength() const -> int
		{
			return Win32::GetWindowTextLengthW(hwnd_);
		}

		void OnChange(std::function<void(const ChangeEvent&)> handler)
		{
			RequireEvents().onChange(GetControlId(), std::move(handler));
		}

		void OnFocus(std::function<void(const FocusEvent&)> handler)
		{
			RequireEvents().onFocus(GetControlId(), std::move(handler));
		}

		void OnBlur(std::function<void(const BlurEvent&)> handler)
		{
			RequireEvents().onBlur(GetControlId(), std::move(handler));
		}
	};

	// RichEdit control wrapper.
	struct RichEdit : ControlBase
	{
		using ControlBase::ControlBase;

		void SetReadOnly(bool readOnly = true)
		{
			Win32::SendMessageW(hwnd_, Win32::EditControl::SetReadOnly,
				readOnly ? 1 : 0, 0);
		}

		void SelectAll()
		{
			Win32::SendMessageW(hwnd_, Win32::EditControl::SetSel, 0, -1);
		}

		auto GetTextLength() const -> int
		{
			return Win32::GetWindowTextLengthW(hwnd_);
		}

		void OnChange(std::function<void(const ChangeEvent&)> handler)
		{
			RequireEvents().onChange(GetControlId(), std::move(handler));
		}

		void OnFocus(std::function<void(const FocusEvent&)> handler)
		{
			RequireEvents().onFocus(GetControlId(), std::move(handler));
		}

		void OnBlur(std::function<void(const BlurEvent&)> handler)
		{
			RequireEvents().onBlur(GetControlId(), std::move(handler));
		}
	};

	// CheckBox control wrapper.
	struct CheckBox : ControlBase
	{
		using ControlBase::ControlBase;

		auto IsChecked() const -> bool
		{
			return Win32::SendMessageW(hwnd_, Win32::Button::GetCheck, 0, 0)
				== Win32::Button::Checked;
		}

		void SetChecked(bool checked = true)
		{
			Win32::SendMessageW(hwnd_, Win32::Button::SetCheck,
				checked ? Win32::Button::Checked : Win32::Button::Unchecked, 0);
		}

		void Toggle()
		{
			SetChecked(!IsChecked());
		}

		void OnClick(std::function<void(const ClickEvent&)> handler)
		{
			RequireEvents().onClick(GetControlId(), std::move(handler));
		}

		void OnCheck(std::function<void(const CheckEvent&)> handler)
		{
			RequireEvents().onCheck(GetControlId(), std::move(handler));
		}

		void OnFocus(std::function<void(const FocusEvent&)> handler)
		{
			RequireEvents().onFocus(GetControlId(), std::move(handler));
		}

		void OnBlur(std::function<void(const BlurEvent&)> handler)
		{
			RequireEvents().onBlur(GetControlId(), std::move(handler));
		}
	};

	// RadioButton control wrapper.
	struct RadioButton : ControlBase
	{
		using ControlBase::ControlBase;

		auto IsSelected() const -> bool
		{
			return Win32::SendMessageW(hwnd_, Win32::Button::GetCheck, 0, 0)
				== Win32::Button::Checked;
		}

		void SetSelected(bool selected = true)
		{
			Win32::SendMessageW(hwnd_, Win32::Button::SetCheck,
				selected ? Win32::Button::Checked : Win32::Button::Unchecked, 0);
		}

		void OnClick(std::function<void(const ClickEvent&)> handler)
		{
			RequireEvents().onClick(GetControlId(), std::move(handler));
		}

		void OnCheck(std::function<void(const CheckEvent&)> handler)
		{
			RequireEvents().onCheck(GetControlId(), std::move(handler));
		}

		void OnFocus(std::function<void(const FocusEvent&)> handler)
		{
			RequireEvents().onFocus(GetControlId(), std::move(handler));
		}

		void OnBlur(std::function<void(const BlurEvent&)> handler)
		{
			RequireEvents().onBlur(GetControlId(), std::move(handler));
		}
	};

	// ComboBox control wrapper.
	struct ComboBox : ControlBase
	{
		using ControlBase::ControlBase;

		void AddItem(std::wstring_view text)
		{
			Win32::SendMessageW(hwnd_, Win32::ComboBox::AddString, 0,
				reinterpret_cast<Win32::LPARAM>(std::wstring{text}.c_str()));
		}

		void InsertItem(int index, std::wstring_view text)
		{
			Win32::SendMessageW(hwnd_, Win32::ComboBox::InsertString, index,
				reinterpret_cast<Win32::LPARAM>(std::wstring{text}.c_str()));
		}

		void RemoveItem(int index)
		{
			Win32::SendMessageW(hwnd_, Win32::ComboBox::DeleteString, index, 0);
		}

		void Clear()
		{
			Win32::SendMessageW(hwnd_, Win32::ComboBox::ResetContent, 0, 0);
		}

		auto GetSelectedIndex() const -> int
		{
			return static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::ComboBox::GetCurSel, 0, 0));
		}

		void SetSelectedIndex(int index)
		{
			Win32::SendMessageW(hwnd_, Win32::ComboBox::SetCurSel, index, 0);
		}

		auto GetCount() const -> int
		{
			return static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::ComboBox::GetCount, 0, 0));
		}

		auto GetItemText(int index) const -> std::wstring
		{
			auto len = static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::ComboBox::GetLBTextLen, index, 0));
			if (len <= 0) return {};
			auto buf = std::wstring(static_cast<size_t>(len) + 1, L'\0');
			Win32::SendMessageW(hwnd_, Win32::ComboBox::GetLBText, index,
				reinterpret_cast<Win32::LPARAM>(buf.data()));
			buf.resize(static_cast<size_t>(len));
			return buf;
		}

		void OnChange(std::function<void(const ChangeEvent&)> handler)
		{
			RequireEvents().onChange(GetControlId(), std::move(handler));
		}

		void OnSelectionChange(std::function<void(const SelectionChangeEvent&)> handler)
		{
			RequireEvents().onSelectionChange(GetControlId(), std::move(handler));
		}

		void OnFocus(std::function<void(const FocusEvent&)> handler)
		{
			RequireEvents().onFocus(GetControlId(), std::move(handler));
		}

		void OnBlur(std::function<void(const BlurEvent&)> handler)
		{
			RequireEvents().onBlur(GetControlId(), std::move(handler));
		}
	};

	// ListBox control wrapper.
	struct ListBox : ControlBase
	{
		using ControlBase::ControlBase;

		void AddItem(std::wstring_view text)
		{
			Win32::SendMessageW(hwnd_, Win32::ListBox::AddString, 0,
				reinterpret_cast<Win32::LPARAM>(std::wstring{text}.c_str()));
		}

		void InsertItem(int index, std::wstring_view text)
		{
			Win32::SendMessageW(hwnd_, Win32::ListBox::InsertString, index,
				reinterpret_cast<Win32::LPARAM>(std::wstring{text}.c_str()));
		}

		void RemoveItem(int index)
		{
			Win32::SendMessageW(hwnd_, Win32::ListBox::DeleteString, index, 0);
		}

		void Clear()
		{
			Win32::SendMessageW(hwnd_, Win32::ListBox::ResetContent, 0, 0);
		}

		auto GetSelectedIndex() const -> int
		{
			return static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::ListBox::GetCurSel, 0, 0));
		}

		void SetSelectedIndex(int index)
		{
			Win32::SendMessageW(hwnd_, Win32::ListBox::SetCurSel, index, 0);
		}

		auto GetCount() const -> int
		{
			return static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::ListBox::GetCount, 0, 0));
		}

		auto GetItemText(int index) const -> std::wstring
		{
			auto len = static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::ListBox::GetTextLen, index, 0));
			if (len <= 0) return {};
			auto buf = std::wstring(static_cast<size_t>(len) + 1, L'\0');
			Win32::SendMessageW(hwnd_, Win32::ListBox::GetText, index,
				reinterpret_cast<Win32::LPARAM>(buf.data()));
			buf.resize(static_cast<size_t>(len));
			return buf;
		}

		void OnDoubleClick(std::function<void(const DoubleClickEvent&)> handler)
		{
			RequireEvents().onDoubleClick(GetControlId(), std::move(handler));
		}

		void OnSelectionChange(std::function<void(const SelectionChangeEvent&)> handler)
		{
			RequireEvents().onSelectionChange(GetControlId(), std::move(handler));
		}

		void OnFocus(std::function<void(const FocusEvent&)> handler)
		{
			RequireEvents().onFocus(GetControlId(), std::move(handler));
		}

		void OnBlur(std::function<void(const BlurEvent&)> handler)
		{
			RequireEvents().onBlur(GetControlId(), std::move(handler));
		}
	};

	// ProgressBar control wrapper.
	struct ProgressBar : ControlBase
	{
		using ControlBase::ControlBase;

		void SetValue(int value)
		{
			Win32::SendMessageW(hwnd_, Win32::ProgressBarMsg::SetPos, value, 0);
		}

		auto GetValue() const -> int
		{
			return static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::ProgressBarMsg::GetPos, 0, 0));
		}

		void SetRange(int min, int max)
		{
			Win32::SendMessageW(hwnd_, Win32::ProgressBarMsg::SetRange32, min, max);
		}

		void Step()
		{
			Win32::SendMessageW(hwnd_, Win32::ProgressBarMsg::StepIt, 0, 0);
		}

		void SetStepSize(int step)
		{
			Win32::SendMessageW(hwnd_, Win32::ProgressBarMsg::SetStep, step, 0);
		}
	};

	// TrackBar (slider) control wrapper.
	struct TrackBar : ControlBase
	{
		using ControlBase::ControlBase;

		void SetValue(int value)
		{
			Win32::SendMessageW(hwnd_, Win32::TrackBarMsg::SetPos, true, value);
		}

		auto GetValue() const -> int
		{
			return static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::TrackBarMsg::GetPos, 0, 0));
		}

		void SetRange(int min, int max)
		{
			Win32::SendMessageW(hwnd_, Win32::TrackBarMsg::SetRangeMin, false, min);
			Win32::SendMessageW(hwnd_, Win32::TrackBarMsg::SetRangeMax, true, max);
		}
	};

	// UpDown (spinner) control wrapper.
	struct UpDown : ControlBase
	{
		using ControlBase::ControlBase;

		void SetValue(int value)
		{
			Win32::SendMessageW(hwnd_, Win32::UpDownMsg::SetPos32, 0, value);
		}

		auto GetValue() const -> int
		{
			return static_cast<int>(
				Win32::SendMessageW(hwnd_, Win32::UpDownMsg::GetPos32, 0, 0));
		}

		void SetRange(int min, int max)
		{
			Win32::SendMessageW(hwnd_, Win32::UpDownMsg::SetRange32, min, max);
		}
	};

	// DateTimePicker control wrapper.
	struct DateTimePicker : ControlBase
	{
		using ControlBase::ControlBase;

		void OnChange(std::function<void(const ChangeEvent&)> handler)
		{
			RequireEvents().onChange(GetControlId(), std::move(handler));
		}
	};

	// FormWindow wraps the top-level form HWND returned by LoadForm.
	struct FormWindow
	{
		FormWindow() = default;
		explicit FormWindow(Win32::HWND h) : hwnd_{h}
		{
			// Retrieve EventMap from FormWindowData stored in window user data.
			auto* ptr = reinterpret_cast<void**>(
				Win32::GetWindowLongPtrW(h, Win32::Gwlp_UserData));
			if (ptr)
				events_ = static_cast<EventMap*>(*ptr);
		}

		auto Handle() const noexcept -> Win32::HWND { return hwnd_; }
		explicit operator bool() const noexcept { return hwnd_ != nullptr; }

		// Typed control access by ID.
		template<typename T>
			requires std::derived_from<T, ControlBase>
		auto Get(int id) const -> T
		{
			return T{Win32::GetDlgItem(hwnd_, id), events_};
		}

		// Convenience accessors.
		auto GetButton(int id) const          -> Button          { return Get<Button>(id); }
		auto GetTextBox(int id) const         -> TextBox         { return Get<TextBox>(id); }
		auto GetCheckBox(int id) const        -> CheckBox        { return Get<CheckBox>(id); }
		auto GetRadioButton(int id) const     -> RadioButton     { return Get<RadioButton>(id); }
		auto GetLabel(int id) const           -> Label           { return Get<Label>(id); }
		auto GetComboBox(int id) const        -> ComboBox        { return Get<ComboBox>(id); }
		auto GetListBox(int id) const         -> ListBox         { return Get<ListBox>(id); }
		auto GetProgressBar(int id) const     -> ProgressBar     { return Get<ProgressBar>(id); }
		auto GetTrackBar(int id) const        -> TrackBar        { return Get<TrackBar>(id); }
		auto GetUpDown(int id) const          -> UpDown          { return Get<UpDown>(id); }
		auto GetDateTimePicker(int id) const  -> DateTimePicker  { return Get<DateTimePicker>(id); }
		auto GetRichEdit(int id) const        -> RichEdit        { return Get<RichEdit>(id); }

		// Form-level operations.
		void Show()    { Win32::ShowWindow(hwnd_, Win32::Sw_Show); }
		void Hide()    { Win32::ShowWindow(hwnd_, Win32::Sw_Hide); }
		void Enable()  { Win32::EnableWindow(hwnd_, true); }
		void Disable() { Win32::EnableWindow(hwnd_, false); }
		void Close()   { Win32::DestroyWindow(hwnd_); }

		auto IsVisible() const -> bool { return Win32::IsWindowVisible(hwnd_) != 0; }
		auto IsEnabled() const -> bool { return Win32::IsWindowEnabled(hwnd_) != 0; }

		void SetTitle(std::wstring_view title)
		{
			Win32::SetWindowTextW(hwnd_, std::wstring{title}.c_str());
		}

		auto GetTitle() const -> std::wstring
		{
			auto len = Win32::GetWindowTextLengthW(hwnd_);
			if (len <= 0) return {};
			auto buf = std::wstring(static_cast<size_t>(len) + 1, L'\0');
			Win32::GetWindowTextW(hwnd_, buf.data(), len + 1);
			buf.resize(static_cast<size_t>(len));
			return buf;
		}

		// Form-level event binding (by control ID).
		void OnClick(int id, std::function<void(const ClickEvent&)> handler)
		{
			if (events_) events_->onClick(id, std::move(handler));
		}

		void OnChange(int id, std::function<void(const ChangeEvent&)> handler)
		{
			if (events_) events_->onChange(id, std::move(handler));
		}

		void OnDoubleClick(int id, std::function<void(const DoubleClickEvent&)> handler)
		{
			if (events_) events_->onDoubleClick(id, std::move(handler));
		}

		void OnSelectionChange(int id, std::function<void(const SelectionChangeEvent&)> handler)
		{
			if (events_) events_->onSelectionChange(id, std::move(handler));
		}

		void OnFocus(int id, std::function<void(const FocusEvent&)> handler)
		{
			if (events_) events_->onFocus(id, std::move(handler));
		}

		void OnBlur(int id, std::function<void(const BlurEvent&)> handler)
		{
			if (events_) events_->onBlur(id, std::move(handler));
		}

		void OnCheck(int id, std::function<void(const CheckEvent&)> handler)
		{
			if (events_) events_->onCheck(id, std::move(handler));
		}

	private:
		Win32::HWND hwnd_ = nullptr;
		EventMap* events_ = nullptr;
	};
}
