export module formbuilder:parser;
import std;
import :win32;
import :json;
import :schema;

export namespace FormDesigner
{
	auto ParseControlType(const std::string& type) -> ControlType
	{
		static const auto map = std::unordered_map<std::string, ControlType>{
			{"Window",      ControlType::Window},
			{"Button",      ControlType::Button},
			{"CheckBox",    ControlType::CheckBox},
			{"RadioButton", ControlType::RadioButton},
			{"Label",       ControlType::Label},
			{"TextBox",     ControlType::TextBox},
			{"GroupBox",    ControlType::GroupBox},
			{"ListBox",     ControlType::ListBox},
			{"ComboBox",    ControlType::ComboBox},
			{"ProgressBar",     ControlType::ProgressBar},
			{"TrackBar",        ControlType::TrackBar},
			{"DateTimePicker",  ControlType::DateTimePicker},
			{"TabControl",      ControlType::TabControl},
			{"ListView",        ControlType::ListView},
			{"TreeView",        ControlType::TreeView},
			{"UpDown",          ControlType::UpDown},
			{"RichEdit",        ControlType::RichEdit},
			{"MonthCalendar",   ControlType::MonthCalendar},
			{"Link",            ControlType::Link},
			{"IPAddress",       ControlType::IPAddress},
			{"HotKey",          ControlType::HotKey},
			{"Picture",         ControlType::Picture},
			{"Separator",       ControlType::Separator},
			{"Animation",       ControlType::Animation},
		};

		if (auto it = map.find(type); it != map.end())
			return it->second;

		throw std::runtime_error(std::format("Unknown control type: '{}'", type));
	}

	auto ParseControl(const nlohmann::json& j) -> Control
	{
		auto control = Control{};

		control.type = ParseControlType(j.at("type").get<std::string>());

		if (j.contains("text"))
		{
			auto narrow = j["text"].get<std::string>();
			control.text = std::wstring(narrow.begin(), narrow.end());
		}

		if (j.contains("rect"))
		{
			auto& r = j["rect"];
			control.rect = Rect{
				.x = r[0].get<int>(),
				.y = r[1].get<int>(),
				.width = r[2].get<int>(),
				.height = r[3].get<int>(),
			};
		}

		if (j.contains("id"))
			control.id = j["id"].get<int>();

		if (j.contains("style"))
			control.style = j["style"].get<Win32::DWORD>();

		if (j.contains("exStyle"))
			control.exStyle = j["exStyle"].get<Win32::DWORD>();

		if (j.contains("onClick"))
			control.onClick = j["onClick"].get<std::string>();

		if (j.contains("onChange"))
			control.onChange = j["onChange"].get<std::string>();

		if (j.contains("onDoubleClick"))
			control.onDoubleClick = j["onDoubleClick"].get<std::string>();

		if (j.contains("onSelectionChange"))
			control.onSelectionChange = j["onSelectionChange"].get<std::string>();

		if (j.contains("onFocus"))
			control.onFocus = j["onFocus"].get<std::string>();

		if (j.contains("onBlur"))
			control.onBlur = j["onBlur"].get<std::string>();

		if (j.contains("onCheck"))
			control.onCheck = j["onCheck"].get<std::string>();

		if (j.contains("tabIndex"))
			control.tabIndex = j["tabIndex"].get<int>();

		if (j.contains("textAlign"))
		{
			auto align = j["textAlign"].get<std::string>();
			if (align == "center")     control.textAlign = TextAlign::Center;
			else if (align == "right") control.textAlign = TextAlign::Right;
		}

		if (j.contains("visible"))
			control.visible = j["visible"].get<bool>();

		if (j.contains("locked"))
			control.locked = j["locked"].get<bool>();

		if (j.contains("groupId"))
			control.groupId = j["groupId"].get<int>();

		if (j.contains("anchor"))
		{
			control.anchor = 0;
			for (auto& flag : j["anchor"])
			{
				auto s = flag.get<std::string>();
				if (s == "top")         control.anchor |= Anchor::Top;
				else if (s == "bottom") control.anchor |= Anchor::Bottom;
				else if (s == "left")   control.anchor |= Anchor::Left;
				else if (s == "right")  control.anchor |= Anchor::Right;
			}
		}

		if (j.contains("font"))
		{
			auto& fj = j["font"];
			if (fj.contains("family"))
			{
				auto narrow = fj["family"].get<std::string>();
				control.font.family = std::wstring(narrow.begin(), narrow.end());
			}
			if (fj.contains("size"))
				control.font.size = fj["size"].get<int>();
			if (fj.contains("bold"))
				control.font.bold = fj["bold"].get<bool>();
			if (fj.contains("italic"))
				control.font.italic = fj["italic"].get<bool>();
		}

		if (j.contains("tooltip"))
		{
			auto narrow = j["tooltip"].get<std::string>();
			control.tooltip = std::wstring(narrow.begin(), narrow.end());
		}

		if (j.contains("items"))
			for (auto& item : j["items"])
			{
				auto narrow = item.get<std::string>();
				control.items.emplace_back(narrow.begin(), narrow.end());
			}

		if (j.contains("selectedIndex"))
			control.selectedIndex = j["selectedIndex"].get<int>();

		if (j.contains("validation"))
		{
			auto& vj = j["validation"];
			if (vj.contains("required"))
				control.validation.required = vj["required"].get<bool>();
			if (vj.contains("minLength"))
				control.validation.minLength = vj["minLength"].get<int>();
			if (vj.contains("maxLength"))
				control.validation.maxLength = vj["maxLength"].get<int>();
			if (vj.contains("pattern"))
				control.validation.pattern = vj["pattern"].get<std::string>();
			if (vj.contains("min"))
				control.validation.min = vj["min"].get<int>();
			if (vj.contains("max"))
				control.validation.max = vj["max"].get<int>();
		}

		if (j.contains("children"))
			for (auto& child : j["children"])
				control.children.push_back(ParseControl(child));

		return control;
	}
}

export namespace FormDesigner
{
	// Parses a Form definition from a JSON string.
	auto ParseForm(const std::string& jsonText) -> Form
	{
		auto j = nlohmann::json::parse(jsonText);
		auto form = Form{};

		if (j.contains("title"))
		{
			auto narrow = j["title"].get<std::string>();
			form.title = std::wstring(narrow.begin(), narrow.end());
		}

		if (j.contains("width"))
			form.width = j["width"].get<int>();

		if (j.contains("height"))
			form.height = j["height"].get<int>();

		if (j.contains("style"))
			form.style = j["style"].get<Win32::DWORD>();

		if (j.contains("exStyle"))
			form.exStyle = j["exStyle"].get<Win32::DWORD>();

		if (j.contains("backgroundColor"))
		{
			auto hex = j["backgroundColor"].get<std::string>();
			if (hex.size() == 7 && hex[0] == '#')
			{
				unsigned int r = std::stoul(hex.substr(1, 2), nullptr, 16);
				unsigned int g = std::stoul(hex.substr(3, 2), nullptr, 16);
				unsigned int b = std::stoul(hex.substr(5, 2), nullptr, 16);
				form.backgroundColor = static_cast<int>(r | (g << 8) | (b << 16));
			}
		}

		if (j.contains("controls"))
			for (auto& control : j["controls"])
				form.controls.push_back(ParseControl(control));

		if (j.contains("guides"))
			for (auto& g : j["guides"])
				form.guides.push_back({
					g.value("horizontal", false),
					g.value("position", 0)
				});

		if (j.contains("font"))
		{
			auto& fj = j["font"];
			if (fj.contains("family"))
			{
				auto narrow = fj["family"].get<std::string>();
				form.font.family = std::wstring(narrow.begin(), narrow.end());
			}
			if (fj.contains("size"))
				form.font.size = fj["size"].get<int>();
			if (fj.contains("bold"))
				form.font.bold = fj["bold"].get<bool>();
			if (fj.contains("italic"))
				form.font.italic = fj["italic"].get<bool>();
		}

		return form;
	}

	// Loads a Form definition from a JSON file on disk.
	auto LoadFormFromFile(const std::filesystem::path& path) -> Form
	{
		auto file = std::ifstream{ path };
		if (not file.is_open())
			throw std::runtime_error(std::format("Cannot open form file: '{}'", path.string()));

		auto content = std::string{
			std::istreambuf_iterator<char>{file},
			std::istreambuf_iterator<char>{}
		};

		return ParseForm(content);
	}
}
