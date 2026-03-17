export module formbuilder:loader;
import std;
import :win32;
import :schema;
import :events;
import :errors;
import :parser;

export namespace FormDesigner
{
	// Recursively creates child controls for a given parent HWND.
	void CreateChildren(
		Win32::HWND parent,
		Win32::HINSTANCE hInstance,
		std::span<const Control> controls,
		const FontInfo& formFont,
		std::vector<Win32::HFONT>& createdFonts,
		int dpi,
		Win32::HWND hTooltips = nullptr,
		const std::wstring& formBasePath = L"")
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
				Win32::Styles::Child | ImpliedStyleFor(control.type)
				| AlignmentStyleFor(control.type, control.textAlign) | control.style
			};
			if (control.visible)
				childStyle |= Win32::Styles::Visible;
			if (!control.enabled)
				childStyle |= Win32::Styles::Disabled;
			if (control.tabStop && IsInteractiveControl(control.type))
				childStyle |= Win32::Styles::TabStop;
			if (control.groupStart)
				childStyle |= Win32::Styles::Group;

			auto hwnd = Win32::CreateWindowExW(
				control.exStyle,
				className,
				control.text.c_str(),
				childStyle,
				Win32::ScaleDpi(control.rect.x, dpi),
				Win32::ScaleDpi(control.rect.y, dpi),
				Win32::ScaleDpi(control.rect.width, dpi),
				Win32::ScaleDpi(control.rect.height, dpi),
				parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(control.id)),
				hInstance,
				nullptr
			);

			if (not hwnd)
			{
				std::println(std::cerr, "Failed to create control '{}': error {}",
					ToNarrow(control.text),
					Win32::GetLastError());
				continue;
			}

			// Apply the resolved font (control → form → system default).
			auto resolved = ResolveFont(control.font, formFont);
			if (resolved.family != DefaultFontFamily || resolved.size != DefaultFontSize
				|| resolved.bold || resolved.italic)
			{
				auto hFont = Win32::CreateFontFromInfo(
					resolved.family.c_str(), resolved.size, resolved.bold, resolved.italic, parent);
				createdFonts.push_back(hFont);
				Win32::SendMessageW(hwnd, Win32::Messages::SetFont, reinterpret_cast<Win32::WPARAM>(hFont), 1);
			}
			else
			{
				auto font = static_cast<Win32::HFONT>(Win32::GetStockObject(Win32::DefaultGuiFont));
				Win32::SendMessageW(hwnd, Win32::Messages::SetFont, reinterpret_cast<Win32::WPARAM>(font), 1);
			}

			// Register tooltip if text is specified.
			if (hTooltips && !control.tooltip.empty())
			{
				Win32::TTTOOLINFOW ti = {};
				ti.cbSize = sizeof(Win32::TTTOOLINFOW);
				ti.uFlags = Win32::TooltipFlags::Subclass | Win32::TooltipFlags::IdIsHwnd;
				ti.hwnd = parent;
				ti.uId = reinterpret_cast<Win32::UINT_PTR>(hwnd);
				ti.lpszText = const_cast<wchar_t*>(control.tooltip.c_str());
				Win32::SendMessageW(hTooltips, Win32::TooltipMessages::AddTool, 0,
					reinterpret_cast<Win32::LPARAM>(&ti));
			}

			// Populate ComboBox/ListBox items.
			if (!control.items.empty() &&
				(control.type == ControlType::ComboBox || control.type == ControlType::ListBox))
			{
				auto addMsg = (control.type == ControlType::ComboBox)
					? Win32::ComboBox::AddString : Win32::ListBox::AddString;
				for (auto& item : control.items)
					Win32::SendMessageW(hwnd, addMsg, 0,
						reinterpret_cast<Win32::LPARAM>(item.c_str()));
				if (control.selectedIndex >= 0)
				{
					auto selMsg = (control.type == ControlType::ComboBox)
						? Win32::ComboBox::SetCurSel : Win32::ListBox::SetCurSel;
					Win32::SendMessageW(hwnd, selMsg, control.selectedIndex, 0);
				}
			}

			// Load image for Picture controls.
			if (control.type == ControlType::Picture && !control.imagePath.empty())
			{
				auto imgType = ImageTypeFromPath(control.imagePath);
				if (imgType > 0)
				{
					// Resolve relative path against form base directory.
					auto fullPath = control.imagePath;
					if (!formBasePath.empty() && control.imagePath.size() > 1
						&& control.imagePath[1] != L':' && control.imagePath[0] != L'\\')
						fullPath = formBasePath + L"\\" + control.imagePath;

					auto winType = (imgType == 1) ? Win32::ImageBitmap : Win32::ImageIcon;
					auto flags = Win32::LoadFromFile | Win32::LoadDefaultSize;
					auto hImg = Win32::LoadImageW(nullptr, fullPath.c_str(),
						static_cast<Win32::UINT>(winType), 0, 0, flags);
					if (hImg)
					{
						// Switch style from SS_ETCHEDFRAME to SS_BITMAP/SS_ICON.
						auto curStyle = static_cast<Win32::DWORD>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwl_Style));
						curStyle &= ~Win32::Styles::StaticEtchedFrame;
						curStyle |= (imgType == 1) ? Win32::Styles::StaticBitmap : Win32::Styles::StaticIcon;
						curStyle |= Win32::Styles::StaticCenterImage;
						Win32::SetWindowLongPtrW(hwnd, Win32::Gwl_Style, curStyle);
						Win32::SendMessageW(hwnd, Win32::StaticMessages::SetImage,
							static_cast<Win32::WPARAM>(winType),
							reinterpret_cast<Win32::LPARAM>(hImg));
					}
				}
			}

			// Set range and value for numeric controls.
			if (SupportsValue(control.type))
			{
				if (control.validation.min != 0 || control.validation.max != 0)
				{
					switch (control.type)
					{
					case ControlType::ProgressBar:
						Win32::SendMessageW(hwnd, Win32::ProgressBarMsg::SetRange32,
							control.validation.min, control.validation.max);
						break;
					case ControlType::TrackBar:
						Win32::SendMessageW(hwnd, Win32::TrackBarMsg::SetRangeMin, false, control.validation.min);
						Win32::SendMessageW(hwnd, Win32::TrackBarMsg::SetRangeMax, true, control.validation.max);
						break;
					case ControlType::UpDown:
						Win32::SendMessageW(hwnd, Win32::UpDownMsg::SetRange32,
							control.validation.min, control.validation.max);
						break;
					default: break;
					}
				}
				if (control.value != 0)
				{
					switch (control.type)
					{
					case ControlType::ProgressBar:
						Win32::SendMessageW(hwnd, Win32::ProgressBarMsg::SetPos, control.value, 0);
						break;
					case ControlType::TrackBar:
						Win32::SendMessageW(hwnd, Win32::TrackBarMsg::SetPos, true, control.value);
						break;
					case ControlType::UpDown:
						Win32::SendMessageW(hwnd, Win32::UpDownMsg::SetPos32, 0, control.value);
						break;
					default: break;
					}
				}
			}

			if (not control.children.empty())
				CreateChildren(hwnd, hInstance, control.children, formFont, createdFonts, dpi, hTooltips, formBasePath);
		}
	}

	// Creates a shared tooltip window as a child of the given parent.
	Win32::HWND CreateTooltipWindow(Win32::HWND parent, Win32::HINSTANCE hInstance)
	{
		auto hTooltips = Win32::CreateWindowExW(
			0, Win32::Controls::Tooltips, nullptr,
			Win32::TooltipStyles::AlwaysTip | Win32::TooltipStyles::NoPrefix,
			0, 0, 0, 0,
			parent, nullptr, hInstance, nullptr);

		if (hTooltips)
			Win32::SendMessageW(hTooltips, Win32::TooltipMessages::SetMaxTipWidth, 0, DefaultTooltipWidth);

		return hTooltips;
	}

	// Stores original position and anchor flags for a control, keyed by control ID.
	struct AnchorInfo
	{
		int id = 0;
		Rect originalRect;
		int anchor = Anchor::Default;
	};

	struct FormWindowData
	{
		EventMap* events = nullptr;
		Win32::HBRUSH bgBrush = nullptr;
		int originalWidth = 0;
		int originalHeight = 0;
		std::vector<AnchorInfo> anchors;
		std::vector<Win32::HFONT> createdFonts;
		Win32::HWND hTooltips = nullptr;
		bool isModal = false;
		int dialogResult = 0;
		Win32::HINSTANCE hInstance = nullptr;
		std::filesystem::path hotReloadPath;
		std::wstring hotReloadBasePath;
		std::filesystem::file_time_type lastWriteTime;
		Win32::UINT_PTR hotReloadTimerId = 0;
	};

	// Checks whether any control (recursively) has tooltip text.
	auto HasTooltips(std::span<const Control> ctrls) -> bool
	{
		for (auto& c : ctrls)
		{
			if (!c.tooltip.empty()) return true;
			if (!c.children.empty() && HasTooltips(c.children)) return true;
		}
		return false;
	}

	// Rebuilds the form window contents from a new Form definition.
	// Destroys all child controls, cleans up resources, and recreates everything.
	void ReloadFormContents(Win32::HWND hwnd, FormWindowData* data, const Form& form)
	{
		// Destroy all child windows.
		while (auto child = Win32::GetWindow(hwnd, Win32::Gw_Child))
			Win32::DestroyWindow(child);

		// Clean up old fonts.
		for (auto hFont : data->createdFonts)
			Win32::DeleteObject(hFont);
		data->createdFonts.clear();

		// Clean up old tooltip window.
		if (data->hTooltips)
		{
			Win32::DestroyWindow(data->hTooltips);
			data->hTooltips = nullptr;
		}

		// Update background brush.
		if (data->bgBrush)
		{
			Win32::DeleteObject(data->bgBrush);
			data->bgBrush = nullptr;
		}
		if (form.backgroundColor != -1)
			data->bgBrush = Win32::CreateSolidBrush(static_cast<Win32::DWORD>(form.backgroundColor));

		// Update window title.
		Win32::SetWindowTextW(hwnd, form.title.c_str());

		// Resize window if dimensions changed.
		auto dpi = static_cast<Win32::UINT>(Win32::GetDpiForSystem());
		if (form.width != data->originalWidth || form.height != data->originalHeight)
		{
			auto rc = Win32::RECT{ 0, 0,
				Win32::ScaleDpi(form.width, dpi),
				Win32::ScaleDpi(form.height, dpi) };

			auto style = static_cast<Win32::DWORD>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwl_Style));
			auto exStyle = static_cast<Win32::DWORD>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwl_ExStyle));
			Win32::AdjustWindowRectExForDpi(&rc, style, 0, exStyle, dpi);

			Win32::SetWindowPos(hwnd, nullptr, 0, 0,
				rc.right - rc.left, rc.bottom - rc.top,
				Win32::Swp::NoMove | Win32::Swp::NoZOrder | Win32::Swp::NoActivate);

			data->originalWidth = form.width;
			data->originalHeight = form.height;
		}

		// Recreate tooltip window if needed.
		if (HasTooltips(form.controls))
			data->hTooltips = CreateTooltipWindow(hwnd, data->hInstance);

		// Recreate child controls.
		CreateChildren(hwnd, data->hInstance, form.controls, form.font, data->createdFonts,
			static_cast<int>(dpi), data->hTooltips, data->hotReloadBasePath);

		// Rebuild anchor list.
		data->anchors.clear();
		for (auto& c : form.controls)
			if (c.id != 0)
				data->anchors.push_back({ c.id, c.rect, c.anchor });

		// Update enabled state.
		Win32::EnableWindow(hwnd, form.enabled);

		// Repaint.
		Win32::InvalidateRect(hwnd, nullptr, true);
	}

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
					// Handler may have destroyed this window (e.g. EndModal, DestroyWindow).
					if (!Win32::IsWindow(hwnd)) return 0;
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

			// onChange: DTN_DATETIMECHANGE, MCN_SELCHANGE, IPN_FIELDCHANGED
			if (nmhdr->code == Win32::NotifyCodes::DateTimeChange ||
				nmhdr->code == Win32::NotifyCodes::MonthCalSelChange ||
				nmhdr->code == Win32::NotifyCodes::IPAddressFieldChange)
			{
				if (auto* handler = data->events->findChangeHandler(controlId))
				{
					(*handler)({ controlId, controlHwnd, hwnd });
					return 0;
				}
			}

			// onClick: NM_CLICK (Link/SysLink)
			if (nmhdr->code == Win32::NotifyCodes::Click)
			{
				if (auto* handler = data->events->findClickHandler(controlId))
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
		case Win32::Messages::Size:
		{
			if (!data || data->anchors.empty())
				return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);

			auto rc = Win32::RECT{};
			Win32::GetClientRect(hwnd, &rc);
			int newW = rc.right;
			int newH = rc.bottom;
			int deltaW = newW - data->originalWidth;
			int deltaH = newH - data->originalHeight;

			for (auto& ai : data->anchors)
			{
				auto childHwnd = Win32::GetDlgItem(hwnd, ai.id);
				if (!childHwnd) continue;

				int x = ai.originalRect.x;
				int y = ai.originalRect.y;
				int w = ai.originalRect.width;
				int h = ai.originalRect.height;

				bool anchorL = (ai.anchor & Anchor::Left)   != 0;
				bool anchorR = (ai.anchor & Anchor::Right)  != 0;
				bool anchorT = (ai.anchor & Anchor::Top)    != 0;
				bool anchorB = (ai.anchor & Anchor::Bottom) != 0;

				if (anchorL && anchorR)
					w += deltaW;
				else if (anchorR && !anchorL)
					x += deltaW;

				if (anchorT && anchorB)
					h += deltaH;
				else if (anchorB && !anchorT)
					y += deltaH;

				Win32::MoveWindow(childHwnd, x, y, w, h, true);
			}
			return 0;
		}
		case Win32::Messages::Timer:
		{
			if (data && data->hotReloadTimerId != 0
				&& wParam == data->hotReloadTimerId)
			{
				try
				{
					auto currentTime = std::filesystem::last_write_time(data->hotReloadPath);
					if (currentTime != data->lastWriteTime)
					{
						data->lastWriteTime = currentTime;
						auto result = TryLoadFormFromFile(data->hotReloadPath);
						if (result)
							ReloadFormContents(hwnd, data, *result);
					}
				}
				catch (...) {} // Ignore filesystem errors (file locked, etc.)
				return 0;
			}
			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		case Win32::Messages::Close:
		{
			// For modal windows, closing via X button or Escape returns Cancel.
			if (data && data->isModal)
			{
				data->dialogResult = static_cast<int>(DialogResult::Cancel);
				Win32::DestroyWindow(hwnd);
				return 0;
			}
			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		case Win32::Messages::Destroy:
		{
			auto modalResult = 0;
			if (data)
			{
				modalResult = data->dialogResult;
				if (data->hotReloadTimerId != 0)
					Win32::KillTimer(hwnd, data->hotReloadTimerId);
				if (data->bgBrush) Win32::DeleteObject(data->bgBrush);
				for (auto hFont : data->createdFonts)
					Win32::DeleteObject(hFont);
				delete data;
			}
			Win32::PostQuitMessage(modalResult);
			return 0;
		}
		default:
			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
	}

	// Creates and shows a top-level window from a Form definition (nothrow overload).
	// formBasePath is the directory containing the form file, used for resolving relative image paths.
	// parent, if not null, creates an owned window (for use with modal dialogs).
	auto TryLoadForm(const Form& form, Win32::HINSTANCE hInstance, EventMap& events,
		const std::wstring& formBasePath = L"", Win32::HWND parent = nullptr) -> std::expected<Win32::HWND, FormException>
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
				return std::unexpected(FormException(FormErrorCode::ClassRegistrationFailed,
					std::format("Failed to register form class: error {}", Win32::GetLastError())));
			}
			registered = true;
		}

		// Calculate window rect so client area matches requested size (DPI-aware).
		auto dpi = static_cast<Win32::UINT>(Win32::GetDpiForSystem());
		auto rc = Win32::RECT{ 0, 0,
			Win32::ScaleDpi(form.width, dpi),
			Win32::ScaleDpi(form.height, dpi) };
		Win32::AdjustWindowRectExForDpi(&rc, form.style, 0, form.exStyle, dpi);

		auto hwnd = Win32::CreateWindowExW(
			form.exStyle,
			ClassName,
			form.title.c_str(),
			form.style,
			Win32::Cw_UseDefault,
			Win32::Cw_UseDefault,
			rc.right - rc.left,
			rc.bottom - rc.top,
			parent,
			nullptr,
			hInstance,
			nullptr
		);

		if (not hwnd)
		{
			return std::unexpected(FormException(FormErrorCode::WindowCreationFailed,
				std::format("Failed to create form window: error {}", Win32::GetLastError())));
		}

		// Store window data for WndProc dispatch.
		auto* windowData = new FormWindowData{
			.events = &events,
			.bgBrush = form.backgroundColor != -1
				? Win32::CreateSolidBrush(static_cast<Win32::DWORD>(form.backgroundColor))
				: nullptr,
			.originalWidth = form.width,
			.originalHeight = form.height,
			.hInstance = hInstance,
		};
		for (auto& c : form.controls)
		{
			if (c.id != 0)
				windowData->anchors.push_back({ c.id, c.rect, c.anchor });
		}

		// Create shared tooltip window if any control has tooltip text.
		if (HasTooltips(form.controls))
			windowData->hTooltips = CreateTooltipWindow(hwnd, hInstance);

		CreateChildren(hwnd, hInstance, form.controls, form.font, windowData->createdFonts,
			static_cast<int>(dpi), windowData->hTooltips, formBasePath);

		Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, reinterpret_cast<Win32::LONG_PTR>(windowData));

		if (!form.enabled)
			Win32::EnableWindow(hwnd, false);

		if (form.visible)
		{
			Win32::ShowWindow(hwnd, Win32::Sw_ShowDefault);
			Win32::UpdateWindow(hwnd);
		}

		return hwnd;
	}

	// Creates and shows a top-level window from a Form definition (throwing overload).
	auto LoadForm(const Form& form, Win32::HINSTANCE hInstance, EventMap& events,
		const std::wstring& formBasePath = L"", Win32::HWND parent = nullptr) -> Win32::HWND
	{
		auto result = TryLoadForm(form, hInstance, events, formBasePath, parent);
		if (!result)
			throw std::move(result.error());
		return *result;
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

	// Ends a modal dialog, setting its result and destroying the window.
	void EndModal(Win32::HWND dialogHwnd, DialogResult result)
	{
		auto* data = reinterpret_cast<FormWindowData*>(
			Win32::GetWindowLongPtrW(dialogHwnd, Win32::Gwlp_UserData));
		if (data)
			data->dialogResult = static_cast<int>(result);
		Win32::DestroyWindow(dialogHwnd);
	}

	// Shows a form as a modal dialog owned by parent (nothrow overload).
	// Disables the parent, runs a nested message loop, and returns the dialog result
	// after the dialog is dismissed via EndModal() or closed.
	auto TryShowModalForm(const Form& form, Win32::HINSTANCE hInstance,
		EventMap& events, Win32::HWND parent,
		const std::wstring& formBasePath = L"") -> std::expected<DialogResult, FormException>
	{
		// Load with visibility suppressed — we control showing.
		auto modalForm = form;
		modalForm.visible = false;

		auto dialogResult = TryLoadForm(modalForm, hInstance, events, formBasePath, parent);
		if (!dialogResult)
			return std::unexpected(std::move(dialogResult.error()));

		auto dialogHwnd = *dialogResult;

		// Mark as modal.
		auto* data = reinterpret_cast<FormWindowData*>(
			Win32::GetWindowLongPtrW(dialogHwnd, Win32::Gwlp_UserData));
		if (data)
			data->isModal = true;

		// Disable parent and show dialog.
		Win32::EnableWindow(parent, false);
		Win32::ShowWindow(dialogHwnd, Win32::Sw_Show);
		Win32::UpdateWindow(dialogHwnd);

		// Nested message loop — runs until the dialog posts WM_QUIT.
		auto msg = Win32::MSG{};
		while (Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			Win32::TranslateMessage(&msg);
			Win32::DispatchMessageW(&msg);
		}

		// Re-enable parent before retrieving result (avoids focus issues).
		Win32::EnableWindow(parent, true);
		Win32::SetForegroundWindow(parent);

		// Result was passed through PostQuitMessage wParam.
		return static_cast<DialogResult>(msg.wParam);
	}

	// Shows a form as a modal dialog owned by parent (throwing overload).
	auto ShowModalForm(const Form& form, Win32::HINSTANCE hInstance,
		EventMap& events, Win32::HWND parent,
		const std::wstring& formBasePath = L"") -> DialogResult
	{
		auto result = TryShowModalForm(form, hInstance, events, parent, formBasePath);
		if (!result)
			throw std::move(result.error());
		return *result;
	}

	constexpr Win32::UINT_PTR HotReloadTimerId = 42001;

	// Starts polling a form file for changes. When the file is modified,
	// the form window rebuilds its contents automatically.
	void EnableHotReload(Win32::HWND formHwnd, const std::filesystem::path& formPath,
		const std::wstring& basePath = L"", unsigned int intervalMs = 500)
	{
		auto* data = reinterpret_cast<FormWindowData*>(
			Win32::GetWindowLongPtrW(formHwnd, Win32::Gwlp_UserData));
		if (!data) return;

		// Stop existing timer if active.
		if (data->hotReloadTimerId != 0)
			Win32::KillTimer(formHwnd, data->hotReloadTimerId);

		data->hotReloadPath = formPath;
		data->hotReloadBasePath = basePath;

		try
		{
			data->lastWriteTime = std::filesystem::last_write_time(formPath);
		}
		catch (...) {}

		data->hotReloadTimerId = HotReloadTimerId;
		Win32::SetTimer(formHwnd, HotReloadTimerId, intervalMs, nullptr);
	}

	// Stops watching the form file for changes.
	void DisableHotReload(Win32::HWND formHwnd)
	{
		auto* data = reinterpret_cast<FormWindowData*>(
			Win32::GetWindowLongPtrW(formHwnd, Win32::Gwlp_UserData));
		if (!data) return;

		if (data->hotReloadTimerId != 0)
		{
			Win32::KillTimer(formHwnd, data->hotReloadTimerId);
			data->hotReloadTimerId = 0;
		}
		data->hotReloadPath.clear();
		data->hotReloadBasePath.clear();
	}
}
