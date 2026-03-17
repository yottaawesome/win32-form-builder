export module formbuilder:accessibility;
import std;
import :schema;

export namespace FormDesigner
{
	enum class AccessibilityLevel { Warning, Error };

	struct AccessibilityWarning
	{
		AccessibilityLevel level = AccessibilityLevel::Warning;
		int controlId = 0;
		std::string controlDesc;
		std::string message;
	};

	auto CheckAccessibility(const Form& form) -> std::vector<AccessibilityWarning>
	{
		auto warnings = std::vector<AccessibilityWarning>{};

		auto descFor = [](const Control& ctrl) -> std::string {
			auto typeStr = std::string{};
			switch (ctrl.type)
			{
			case ControlType::Button:          typeStr = "Button"; break;
			case ControlType::CheckBox:        typeStr = "CheckBox"; break;
			case ControlType::RadioButton:     typeStr = "RadioButton"; break;
			case ControlType::Label:           typeStr = "Label"; break;
			case ControlType::TextBox:         typeStr = "TextBox"; break;
			case ControlType::GroupBox:        typeStr = "GroupBox"; break;
			case ControlType::ListBox:         typeStr = "ListBox"; break;
			case ControlType::ComboBox:        typeStr = "ComboBox"; break;
			case ControlType::ProgressBar:     typeStr = "ProgressBar"; break;
			case ControlType::TrackBar:        typeStr = "TrackBar"; break;
			case ControlType::DateTimePicker:  typeStr = "DateTimePicker"; break;
			case ControlType::TabControl:      typeStr = "TabControl"; break;
			case ControlType::ListView:        typeStr = "ListView"; break;
			case ControlType::TreeView:        typeStr = "TreeView"; break;
			case ControlType::UpDown:          typeStr = "UpDown"; break;
			case ControlType::RichEdit:        typeStr = "RichEdit"; break;
			case ControlType::MonthCalendar:   typeStr = "MonthCalendar"; break;
			case ControlType::Link:            typeStr = "Link"; break;
			case ControlType::IPAddress:       typeStr = "IPAddress"; break;
			case ControlType::HotKey:          typeStr = "HotKey"; break;
			case ControlType::Picture:         typeStr = "Picture"; break;
			case ControlType::Separator:       typeStr = "Separator"; break;
			case ControlType::Animation:       typeStr = "Animation"; break;
			default:                           typeStr = "Control"; break;
			}
			if (ctrl.id > 0)
				return typeStr + " #" + std::to_string(ctrl.id);
			auto narrowText = std::string(ctrl.text.begin(), ctrl.text.end());
			if (!narrowText.empty())
				return typeStr + " \"" + narrowText + "\"";
			return typeStr;
		};

		auto hasAccessKey = [](const std::wstring& text) -> bool {
			for (size_t i = 0; i + 1 < text.size(); ++i)
				if (text[i] == L'&' && text[i + 1] != L'&')
					return true;
			return false;
		};

		auto& controls = form.controls;

		// Track tabIndex values for duplicate detection.
		auto tabIndices = std::unordered_map<int, int>{}; // tabIndex -> controlId
		// Track whether we've seen a groupStart for radio button grouping.
		bool inRadioGroup = false;

		for (size_t i = 0; i < controls.size(); ++i)
		{
			auto& ctrl = controls[i];
			auto desc = descFor(ctrl);

			// Rule 1: Interactive control without a preceding Label in z-order.
			if (IsInteractiveControl(ctrl.type) && ctrl.accessibleName.empty())
			{
				bool hasPrecedingLabel = false;
				if (i > 0 && controls[i - 1].type == ControlType::Label)
					hasPrecedingLabel = true;
				if (!hasPrecedingLabel)
				{
					warnings.push_back({
						.level = AccessibilityLevel::Warning,
						.controlId = ctrl.id,
						.controlDesc = desc,
						.message = "No preceding label — screen readers may not identify this control"
					});
				}
			}

			// Rule 2: Button without access key.
			if (ctrl.type == ControlType::Button && !ctrl.text.empty() && !hasAccessKey(ctrl.text))
			{
				warnings.push_back({
					.level = AccessibilityLevel::Warning,
					.controlId = ctrl.id,
					.controlDesc = desc,
					.message = "Button text has no access key (&) — no keyboard shortcut"
				});
			}

			// Rule 3: Label without access key.
			if (ctrl.type == ControlType::Label && !ctrl.text.empty() && !hasAccessKey(ctrl.text))
			{
				warnings.push_back({
					.level = AccessibilityLevel::Warning,
					.controlId = ctrl.id,
					.controlDesc = desc,
					.message = "Label text has no access key (&) — no keyboard shortcut"
				});
			}

			// Rule 4: Picture without accessible name.
			if (ctrl.type == ControlType::Picture && ctrl.accessibleName.empty())
			{
				warnings.push_back({
					.level = AccessibilityLevel::Error,
					.controlId = ctrl.id,
					.controlDesc = desc,
					.message = "Picture has no accessible name — invisible to screen readers"
				});
			}

			// Rule 5: Duplicate tab indices.
			if (ctrl.tabIndex > 0)
			{
				auto it = tabIndices.find(ctrl.tabIndex);
				if (it != tabIndices.end())
				{
					warnings.push_back({
						.level = AccessibilityLevel::Error,
						.controlId = ctrl.id,
						.controlDesc = desc,
						.message = "Duplicate tabIndex " + std::to_string(ctrl.tabIndex)
							+ " — also used by control #" + std::to_string(it->second)
					});
				}
				else
				{
					tabIndices[ctrl.tabIndex] = ctrl.id;
				}
			}

			// Rule 6: Interactive control with tabStop disabled.
			if (IsInteractiveControl(ctrl.type) && !ctrl.tabStop)
			{
				warnings.push_back({
					.level = AccessibilityLevel::Warning,
					.controlId = ctrl.id,
					.controlDesc = desc,
					.message = "Tab stop disabled — control not reachable via keyboard"
				});
			}

			// Rule 7: RadioButton not in a group.
			if (ctrl.type == ControlType::RadioButton)
			{
				if (ctrl.groupStart)
					inRadioGroup = true;
				else if (!inRadioGroup)
				{
					warnings.push_back({
						.level = AccessibilityLevel::Warning,
						.controlId = ctrl.id,
						.controlDesc = desc,
						.message = "RadioButton not in a group — add groupStart to the first radio in each group"
					});
				}
			}
			else if (ctrl.groupStart)
			{
				inRadioGroup = false; // Non-radio with groupStart resets group tracking.
			}
		}

		return warnings;
	}
}
