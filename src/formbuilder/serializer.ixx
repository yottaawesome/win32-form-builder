export module formbuilder:serializer;
import std;
import :win32;
import :json;
import :schema;

namespace FormDesigner
{
	auto ControlTypeName(ControlType type) -> std::string
	{
		switch (type)
		{
		case ControlType::Window:   return "Window";
		case ControlType::Button:   return "Button";
		case ControlType::CheckBox: return "CheckBox";
		case ControlType::Label:    return "Label";
		case ControlType::TextBox:  return "TextBox";
		case ControlType::GroupBox: return "GroupBox";
		case ControlType::ListBox:  return "ListBox";
		case ControlType::ComboBox: return "ComboBox";
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

		if (not control.children.empty())
		{
			j["children"] = nlohmann::json::array();
			for (auto& child : control.children)
				j["children"].push_back(SerializeControl(child));
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

		j["controls"] = nlohmann::json::array();
		for (auto& control : form.controls)
			j["controls"].push_back(SerializeControl(control));

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
