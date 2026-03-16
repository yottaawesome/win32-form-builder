export module formbuilder:loader;
import std;
import :win32;
import :schema;
import :events;

export namespace FormDesigner
{
	// Recursively creates child controls for a given parent HWND.
	void CreateChildren(
		Win32::HWND parent,
		Win32::HINSTANCE hInstance,
		std::span<const Control> controls)
	{
		for (auto& control : controls)
		{
			const auto* className = ClassNameFor(control.type);
			if (not className)
			{
				std::println(std::cerr, "Unknown control type, skipping");
				continue;
			}

			auto childStyle = Win32::DWORD{
				Win32::Styles::Child | Win32::Styles::Visible | ImpliedStyleFor(control.type)
				| AlignmentStyleFor(control.type, control.textAlign) | control.style
			};

			auto hwnd = Win32::CreateWindowExW(
				control.exStyle,
				className,
				control.text.c_str(),
				childStyle,
				control.rect.x,
				control.rect.y,
				control.rect.width,
				control.rect.height,
				parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(control.id)),
				hInstance,
				nullptr
			);

			if (not hwnd)
			{
				std::println(std::cerr, "Failed to create control '{}': error {}",
					std::string(control.text.begin(), control.text.end()),
					Win32::GetLastError());
				continue;
			}

			// Set the default GUI font on the control.
			auto font = static_cast<Win32::HFONT>(Win32::GetStockObject(Win32::DefaultGuiFont));
			Win32::SendMessageW(hwnd, Win32::Messages::SetFont, reinterpret_cast<Win32::WPARAM>(font), 1);

			if (not control.children.empty())
				CreateChildren(hwnd, hInstance, control.children);
		}
	}

	struct FormWindowData
	{
		const EventMap* events = nullptr;
		Win32::HBRUSH bgBrush = nullptr;
	};

	// Window procedure for designer-created forms.
	auto __stdcall FormWndProc(Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
	{
		auto* data = reinterpret_cast<FormWindowData*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

		switch (msg)
		{
		case Win32::Messages::EraseBkgnd:
		{
			if (data && data->bgBrush)
			{
				auto hdc = reinterpret_cast<Win32::HDC>(wParam);
				auto rc = Win32::RECT{};
				Win32::GetClientRect(hwnd, &rc);
				Win32::FillRect(hdc, &rc, data->bgBrush);
				return 1;
			}
			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		case Win32::Messages::Command:
		{
			if (!data || !data->events)
				return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);

			auto notificationCode = Win32::GetHighWord(wParam);
			auto controlId = static_cast<int>(Win32::GetLowWord(wParam));
			auto controlHwnd = reinterpret_cast<Win32::HWND>(lParam);

			// onClick: BN_CLICKED (also triggers onCheck for CheckBox/RadioButton)
			if (notificationCode == Win32::Notifications::ButtonClicked)
			{
				if (auto* handler = data->events->findClickHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
				}
				// onCheck: query check state after BN_CLICKED on checkable controls
				if (auto* handler = data->events->findCheckHandler(controlId))
				{
					auto checkState = Win32::SendMessageW(controlHwnd, Win32::Button::GetCheck, 0, 0);
					(*handler)({ controlId, controlHwnd, hwnd, checkState == Win32::Button::Checked });
				}
				return 0;
			}

			// onFocus: EN_SETFOCUS, LBN_SETFOCUS, CBN_SETFOCUS
			if (notificationCode == Win32::Notifications::EditSetFocus ||
				notificationCode == Win32::Notifications::ListBoxSetFocus ||
				notificationCode == Win32::Notifications::ComboBoxSetFocus)
			{
				if (auto* handler = data->events->findFocusHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
					return 0;
				}
			}

			// onBlur: EN_KILLFOCUS, LBN_KILLFOCUS, CBN_KILLFOCUS
			if (notificationCode == Win32::Notifications::EditKillFocus ||
				notificationCode == Win32::Notifications::ListBoxKillFocus ||
				notificationCode == Win32::Notifications::ComboBoxKillFocus)
			{
				if (auto* handler = data->events->findBlurHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
					return 0;
				}
			}

			// onChange: EN_CHANGE, CBN_SELCHANGE, LBN_SELCHANGE
			if (notificationCode == Win32::Notifications::EditChange ||
				notificationCode == Win32::Notifications::ComboBoxSelChange ||
				notificationCode == Win32::Notifications::ListBoxSelChange)
			{
				if (auto* handler = data->events->findChangeHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
					return 0;
				}
			}

			// onDoubleClick: BN_DBLCLK, LBN_DBLCLK
			if (notificationCode == Win32::Notifications::ButtonDoubleClicked ||
				notificationCode == Win32::Notifications::ListBoxDoubleClick)
			{
				if (auto* handler = data->events->findDoubleClickHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
					return 0;
				}
			}

			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		case Win32::Messages::Notify:
		{
			if (!data || !data->events)
				return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);

			auto* nmhdr = reinterpret_cast<Win32::NMHDR*>(lParam);
			auto controlId = static_cast<int>(nmhdr->idFrom);
			auto controlHwnd = nmhdr->hwndFrom;

			// onDoubleClick: NM_DBLCLK (ListView, TreeView)
			if (nmhdr->code == Win32::NotifyCodes::DoubleClick)
			{
				if (auto* handler = data->events->findDoubleClickHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
					return 0;
				}
			}

			// onChange: DTN_DATETIMECHANGE
			if (nmhdr->code == Win32::NotifyCodes::DateTimeChange)
			{
				if (auto* handler = data->events->findChangeHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
					return 0;
				}
			}

			// onSelectionChange: TVN_SELCHANGED, LVN_ITEMCHANGED, TCN_SELCHANGE
			if (nmhdr->code == Win32::NotifyCodes::TreeViewSelChanged ||
				nmhdr->code == Win32::NotifyCodes::ListViewItemChanged ||
				nmhdr->code == Win32::NotifyCodes::TabSelChange)
			{
				if (auto* handler = data->events->findSelectionChangeHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
					return 0;
				}
			}

			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		case Win32::Messages::Destroy:
		{
			if (data)
			{
				if (data->bgBrush) Win32::DeleteObject(data->bgBrush);
				delete data;
			}
			Win32::PostQuitMessage(0);
			return 0;
		}
		default:
			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
	}

	// Creates and shows a top-level window from a Form definition.
	// Returns the HWND of the created window, or nullptr on failure.
	auto LoadForm(const Form& form, Win32::HINSTANCE hInstance, const EventMap& events) -> Win32::HWND
	{
		static constexpr auto ClassName = L"FormDesignerWindow";
		static auto registered = false;

		if (not registered)
		{
			auto wc = Win32::WNDCLASSEXW{
				.cbSize = sizeof(Win32::WNDCLASSEXW),
				.style = Win32::Cs_HRedraw | Win32::Cs_VRedraw,
				.lpfnWndProc = FormWndProc,
				.hInstance = hInstance,
				.hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
				.hbrBackground = reinterpret_cast<Win32::HBRUSH>(Win32::ColorWindow + 1),
				.lpszClassName = ClassName,
			};

			if (not Win32::RegisterClassExW(&wc))
			{
				std::println(std::cerr, "Failed to register form class: error {}", Win32::GetLastError());
				return nullptr;
			}
			registered = true;
		}

		// Calculate window rect so client area matches requested size.
		auto rc = Win32::RECT{ 0, 0, form.width, form.height };
		Win32::AdjustWindowRectEx(&rc, form.style, 0, form.exStyle);

		auto hwnd = Win32::CreateWindowExW(
			form.exStyle,
			ClassName,
			form.title.c_str(),
			form.style,
			Win32::Cw_UseDefault,
			Win32::Cw_UseDefault,
			rc.right - rc.left,
			rc.bottom - rc.top,
			nullptr,
			nullptr,
			hInstance,
			nullptr
		);

		if (not hwnd)
		{
			std::println(std::cerr, "Failed to create form window: error {}", Win32::GetLastError());
			return nullptr;
		}

		CreateChildren(hwnd, hInstance, form.controls);

		// Store window data for WndProc dispatch.
		auto* windowData = new FormWindowData{
			.events = &events,
			.bgBrush = form.backgroundColor != -1
				? Win32::CreateSolidBrush(static_cast<Win32::DWORD>(form.backgroundColor))
				: nullptr,
		};
		Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, reinterpret_cast<Win32::LONG_PTR>(windowData));

		Win32::ShowWindow(hwnd, Win32::Sw_ShowDefault);
		Win32::UpdateWindow(hwnd);

		return hwnd;
	}

	// Runs the message loop. Call after LoadForm.
	auto RunMessageLoop() -> int
	{
		auto msg = Win32::MSG{};
		while (Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			Win32::TranslateMessage(&msg);
			Win32::DispatchMessageW(&msg);
		}
		return static_cast<int>(msg.wParam);
	}
}
