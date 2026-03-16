export module formbuilder:events;
import std;
import :win32;

export namespace FormDesigner
{
	struct ClickEvent
	{
		int controlId;
		Win32::HWND controlHwnd;
		Win32::HWND formHwnd;
	};

	struct ChangeEvent
	{
		int controlId;
		Win32::HWND controlHwnd;
		Win32::HWND formHwnd;
	};

	struct DoubleClickEvent
	{
		int controlId;
		Win32::HWND controlHwnd;
		Win32::HWND formHwnd;
	};

	struct SelectionChangeEvent
	{
		int controlId;
		Win32::HWND controlHwnd;
		Win32::HWND formHwnd;
	};

	struct FocusEvent
	{
		int controlId;
		Win32::HWND controlHwnd;
		Win32::HWND formHwnd;
	};

	struct BlurEvent
	{
		int controlId;
		Win32::HWND controlHwnd;
		Win32::HWND formHwnd;
	};

	struct CheckEvent
	{
		int controlId;
		Win32::HWND controlHwnd;
		Win32::HWND formHwnd;
		bool checked;
	};

	struct EventMap
	{
		void onClick(int id, std::function<void(const ClickEvent&)> h) { m_click[id] = std::move(h); }
		void onChange(int id, std::function<void(const ChangeEvent&)> h) { m_change[id] = std::move(h); }
		void onDoubleClick(int id, std::function<void(const DoubleClickEvent&)> h) { m_dblClick[id] = std::move(h); }
		void onSelectionChange(int id, std::function<void(const SelectionChangeEvent&)> h) { m_selChange[id] = std::move(h); }
		void onFocus(int id, std::function<void(const FocusEvent&)> h) { m_focus[id] = std::move(h); }
		void onBlur(int id, std::function<void(const BlurEvent&)> h) { m_blur[id] = std::move(h); }
		void onCheck(int id, std::function<void(const CheckEvent&)> h) { m_check[id] = std::move(h); }

		auto findClickHandler(int id) const -> const std::function<void(const ClickEvent&)>*
		{ auto it = m_click.find(id); return it != m_click.end() ? &it->second : nullptr; }

		auto findChangeHandler(int id) const -> const std::function<void(const ChangeEvent&)>*
		{ auto it = m_change.find(id); return it != m_change.end() ? &it->second : nullptr; }

		auto findDoubleClickHandler(int id) const -> const std::function<void(const DoubleClickEvent&)>*
		{ auto it = m_dblClick.find(id); return it != m_dblClick.end() ? &it->second : nullptr; }

		auto findSelectionChangeHandler(int id) const -> const std::function<void(const SelectionChangeEvent&)>*
		{ auto it = m_selChange.find(id); return it != m_selChange.end() ? &it->second : nullptr; }

		auto findFocusHandler(int id) const -> const std::function<void(const FocusEvent&)>*
		{ auto it = m_focus.find(id); return it != m_focus.end() ? &it->second : nullptr; }

		auto findBlurHandler(int id) const -> const std::function<void(const BlurEvent&)>*
		{ auto it = m_blur.find(id); return it != m_blur.end() ? &it->second : nullptr; }

		auto findCheckHandler(int id) const -> const std::function<void(const CheckEvent&)>*
		{ auto it = m_check.find(id); return it != m_check.end() ? &it->second : nullptr; }

		private:
		std::unordered_map<int, std::function<void(const ClickEvent&)>> m_click;
		std::unordered_map<int, std::function<void(const ChangeEvent&)>> m_change;
		std::unordered_map<int, std::function<void(const DoubleClickEvent&)>> m_dblClick;
		std::unordered_map<int, std::function<void(const SelectionChangeEvent&)>> m_selChange;
		std::unordered_map<int, std::function<void(const FocusEvent&)>> m_focus;
		std::unordered_map<int, std::function<void(const BlurEvent&)>> m_blur;
		std::unordered_map<int, std::function<void(const CheckEvent&)>> m_check;
	};
}
