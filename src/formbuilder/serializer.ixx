export module formbuilder:serializer;
import std;
import :win32;
import :json;
import :schema;

export namespace FormDesigner
{
	auto ControlTypeName(ControlType type) -> std::string
	{
		switch (type)
		{
		case ControlType::Window:      return "Window";
		case ControlType::Button:      return "Button";
		case ControlType::CheckBox:    return "CheckBox";
		case ControlType::RadioButton: return "RadioButton";
		case ControlType::Label:       return "Label";
		case ControlType::TextBox:     return "TextBox";
		case ControlType::GroupBox:    return "GroupBox";
		case ControlType::ListBox:     return "ListBox";
		case ControlType::ComboBox:    return "ComboBox";
		case ControlType::ProgressBar:     return "ProgressBar";
		case ControlType::TrackBar:        return "TrackBar";
		case ControlType::DateTimePicker:  return "DateTimePicker";
		case ControlType::TabControl:      return "TabControl";
		case ControlType::ListView:        return "ListView";
		case ControlType::TreeView:        return "TreeView";
		case ControlType::UpDown:          return "UpDown";
		case ControlType::RichEdit:        return "RichEdit";
		case ControlType::MonthCalendar:   return "MonthCalendar";
		case ControlType::Link:            return "Link";
		case ControlType::IPAddress:       return "IPAddress";
		case ControlType::HotKey:          return "HotKey";
		case ControlType::Picture:         return "Picture";
		case ControlType::Separator:       return "Separator";
		case ControlType::Animation:       return "Animation";
		default:                    return "Window";
		}
	}

	auto SerializeControl(const Control& control) -> nlohmann::json
	{
		auto j = nlohmann::json{};

		j["type"] = ControlTypeName(control.type);

		if (not control.text.empty())
			j["text"] = std::string(control.text.begin(), control.text.end());

		j["rect"] = { control.rect.x, control.rect.y, control.rect.width, control.rect.height };

		if (control.id != 0)
			j["id"] = control.id;

		if (control.style != 0)
			j["style"] = control.style;

		if (control.exStyle != 0)
			j["exStyle"] = control.exStyle;

		if (not control.onClick.empty())
			j["onClick"] = control.onClick;

		if (not control.onChange.empty())
			j["onChange"] = control.onChange;

		if (not control.onDoubleClick.empty())
			j["onDoubleClick"] = control.onDoubleClick;

		if (not control.onSelectionChange.empty())
			j["onSelectionChange"] = control.onSelectionChange;

		if (not control.onFocus.empty())
			j["onFocus"] = control.onFocus;

		if (not control.onBlur.empty())
			j["onBlur"] = control.onBlur;

		if (not control.onCheck.empty())
			j["onCheck"] = control.onCheck;

		if (control.tabIndex != 0)
			j["tabIndex"] = control.tabIndex;

		if (control.textAlign != TextAlign::Left)
		{
			switch (control.textAlign)
			{
			case TextAlign::Center: j["textAlign"] = "center"; break;
			case TextAlign::Right:  j["textAlign"] = "right";  break;
			default: break;
			}
		}

		if (!control.visible)
			j["visible"] = false;

		if (control.locked)
			j["locked"] = true;

		if (control.groupId != 0)
			j["groupId"] = control.groupId;

		if (control.anchor != Anchor::Default)
		{
			auto arr = nlohmann::json::array();
			if (control.anchor & Anchor::Top)    arr.push_back("top");
			if (control.anchor & Anchor::Bottom) arr.push_back("bottom");
			if (control.anchor & Anchor::Left)   arr.push_back("left");
			if (control.anchor & Anchor::Right)  arr.push_back("right");
			j["anchor"] = arr;
		}

		if (not control.tooltip.empty())
			j["tooltip"] = std::string(control.tooltip.begin(), control.tooltip.end());

		if (!control.items.empty())
		{
			auto arr = nlohmann::json::array();
			for (auto& item : control.items)
				arr.push_back(std::string(item.begin(), item.end()));
			j["items"] = arr;
		}

		if (control.selectedIndex >= 0)
			j["selectedIndex"] = control.selectedIndex;

		if (control.validation.isSet())
		{
			auto vj = nlohmann::json{};
			if (control.validation.required)
				vj["required"] = true;
			if (control.validation.minLength != 0)
				vj["minLength"] = control.validation.minLength;
			if (control.validation.maxLength != 0)
				vj["maxLength"] = control.validation.maxLength;
			if (!control.validation.pattern.empty())
				vj["pattern"] = control.validation.pattern;
			if (control.validation.min != 0)
				vj["min"] = control.validation.min;
			if (control.validation.max != 0)
				vj["max"] = control.validation.max;
			j["validation"] = vj;
		}

		if (!control.imagePath.empty())
			j["imagePath"] = std::string(control.imagePath.begin(), control.imagePath.end());

		if (!control.bindField.empty())
			j["bindField"] = control.bindField;

		if (not control.children.empty())
		{
			j["children"] = nlohmann::json::array();
			for (auto& child : control.children)
				j["children"].push_back(SerializeControl(child));
		}

		if (control.font.isSet())
		{
			auto fj = nlohmann::json{};
			if (!control.font.family.empty())
				fj["family"] = std::string(control.font.family.begin(), control.font.family.end());
			if (control.font.size != 0)
				fj["size"] = control.font.size;
			if (control.font.bold)
				fj["bold"] = true;
			if (control.font.italic)
				fj["italic"] = true;
			j["font"] = fj;
		}

		return j;
	}
}

export namespace FormDesigner
{
	// Serializes a Form to a JSON string.
	auto SerializeForm(const Form& form, int indent = 2) -> std::string
	{
		auto j = nlohmann::json{};

		j["title"] = std::string(form.title.begin(), form.title.end());
		j["width"] = form.width;
		j["height"] = form.height;

		if (form.style != Win32::Styles::OverlappedWindow)
			j["style"] = form.style;

		if (form.exStyle != 0)
			j["exStyle"] = form.exStyle;

		if (form.backgroundColor != -1)
		{
			auto cr = static_cast<unsigned int>(form.backgroundColor);
			j["backgroundColor"] = std::format("#{:02X}{:02X}{:02X}",
				cr & 0xFF, (cr >> 8) & 0xFF, (cr >> 16) & 0xFF);
		}

		j["controls"] = nlohmann::json::array();
		for (auto& control : form.controls)
			j["controls"].push_back(SerializeControl(control));

		if (!form.guides.empty())
		{
			j["guides"] = nlohmann::json::array();
			for (auto& g : form.guides)
			{
				auto gj = nlohmann::json{};
				gj["horizontal"] = g.horizontal;
				gj["position"] = g.position;
				j["guides"].push_back(gj);
			}
		}

		if (form.font.isSet())
		{
			auto fj = nlohmann::json{};
			if (!form.font.family.empty())
				fj["family"] = std::string(form.font.family.begin(), form.font.family.end());
			if (form.font.size != 0)
				fj["size"] = form.font.size;
			if (form.font.bold)
				fj["bold"] = true;
			if (form.font.italic)
				fj["italic"] = true;
			j["font"] = fj;
		}

		if (!form.bindStruct.empty())
			j["bindStruct"] = form.bindStruct;

		return j.dump(indent);
	}

	// Saves a Form definition to a JSON file on disk.
	auto SaveFormToFile(const Form& form, const std::filesystem::path& path) -> void
	{
		auto file = std::ofstream{ path };
		if (not file.is_open())
			throw std::runtime_error(std::format("Cannot open file for writing: '{}'", path.string()));

		file << SerializeForm(form);
	}
}
