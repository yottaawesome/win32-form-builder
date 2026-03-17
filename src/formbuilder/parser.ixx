export module formbuilder:parser;
import std;
import :win32;
import :json;
import :schema;
import :errors;

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

		throw FormException(FormErrorCode::UnknownControlType,
			std::format("Unknown control type: '{}'", type));
	}

	auto ParseControl(const nlohmann::json& j) -> Control
	{
		auto control = Control{};

		control.type = ParseControlType(j.at("type").get<std::string>());

		if (j.contains("text"))
		{
			auto narrow = j["text"].get<std::string>();
			control.text = ToWide(narrow);
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

		if (j.contains("enabled"))
			control.enabled = j["enabled"].get<bool>();

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
				control.font.family = ToWide(fj["family"].get<std::string>());
			if (fj.contains("size"))
				control.font.size = fj["size"].get<int>();
			if (fj.contains("bold"))
				control.font.bold = fj["bold"].get<bool>();
			if (fj.contains("italic"))
				control.font.italic = fj["italic"].get<bool>();
		}

		if (j.contains("tooltip"))
			control.tooltip = ToWide(j["tooltip"].get<std::string>());

		if (j.contains("items"))
			for (auto& item : j["items"])
				control.items.push_back(ToWide(item.get<std::string>()));

		if (j.contains("selectedIndex"))
			control.selectedIndex = j["selectedIndex"].get<int>();

		if (j.contains("value"))
			control.value = j["value"].get<int>();

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

		if (j.contains("imagePath"))
			control.imagePath = ToWide(j["imagePath"].get<std::string>());

		if (j.contains("bindField"))
			control.bindField = j["bindField"].get<std::string>();

		if (j.contains("tabStop"))
			control.tabStop = j["tabStop"].get<bool>();

		if (j.contains("groupStart"))
			control.groupStart = j["groupStart"].get<bool>();

		if (j.contains("accessibleName"))
			control.accessibleName = ToWide(j["accessibleName"].get<std::string>());

		if (j.contains("accessibleDescription"))
			control.accessibleDescription = ToWide(j["accessibleDescription"].get<std::string>());

		if (j.contains("children"))
			for (auto& child : j["children"])
				control.children.push_back(ParseControl(child));

		return control;
	}
}

export namespace FormDesigner
{
	// Parses a Form definition from a JSON string (nothrow overload).
	auto TryParseForm(const std::string& jsonText) -> std::expected<Form, FormException>
	{
		nlohmann::json j;
		try
		{
			j = nlohmann::json::parse(jsonText);
		}
		catch (const nlohmann::json::parse_error& e)
		{
			return std::unexpected(FormException(FormErrorCode::ParseError,
				std::format("JSON parse error: {}", e.what())));
		}

		try
		{
			auto form = Form{};

			if (j.contains("title"))
				form.title = ToWide(j["title"].get<std::string>());

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
				form.backgroundColor = HexToColorRef(hex);
			}

			if (j.contains("visible"))
				form.visible = j["visible"].get<bool>();

			if (j.contains("enabled"))
				form.enabled = j["enabled"].get<bool>();

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
					form.font.family = ToWide(fj["family"].get<std::string>());
				if (fj.contains("size"))
					form.font.size = fj["size"].get<int>();
				if (fj.contains("bold"))
					form.font.bold = fj["bold"].get<bool>();
				if (fj.contains("italic"))
					form.font.italic = fj["italic"].get<bool>();
			}

			if (j.contains("bindStruct"))
				form.bindStruct = j["bindStruct"].get<std::string>();

			return form;
		}
		catch (const FormException& e)
		{
			return std::unexpected(e);
		}
		catch (const nlohmann::json::out_of_range& e)
		{
			return std::unexpected(FormException(FormErrorCode::InvalidField,
				std::format("Missing required field: {}", e.what())));
		}
		catch (const nlohmann::json::type_error& e)
		{
			return std::unexpected(FormException(FormErrorCode::InvalidField,
				std::format("Invalid field type: {}", e.what())));
		}
		catch (const nlohmann::json::exception& e)
		{
			return std::unexpected(FormException(FormErrorCode::ParseError,
				std::format("JSON error: {}", e.what())));
		}
	}

	// Parses a Form definition from a JSON string (throwing overload).
	auto ParseForm(const std::string& jsonText) -> Form
	{
		auto result = TryParseForm(jsonText);
		if (!result)
			throw std::move(result.error());
		return std::move(*result);
	}

	// Loads a Form definition from a JSON file on disk (nothrow overload).
	auto TryLoadFormFromFile(const std::filesystem::path& path) -> std::expected<Form, FormException>
	{
		auto file = std::ifstream{ path };
		if (not file.is_open())
			return std::unexpected(FormException(FormErrorCode::FileNotFound,
				std::format("Cannot open form file: '{}'", path.string())));

		auto content = std::string{
			std::istreambuf_iterator<char>{file},
			std::istreambuf_iterator<char>{}
		};

		return TryParseForm(content);
	}

	// Loads a Form definition from a JSON file on disk (throwing overload).
	auto LoadFormFromFile(const std::filesystem::path& path) -> Form
	{
		auto result = TryLoadFormFromFile(path);
		if (!result)
			throw std::move(result.error());
		return std::move(*result);
	}
}
