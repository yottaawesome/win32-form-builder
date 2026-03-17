export module formbuilder:serializer;
import std;
import :win32;
import :json;
import :schema;
import :errors;

export namespace FormDesigner
{
	auto SerializeControl(const Control& control) -> nlohmann::json
	{
		auto j = nlohmann::json{};

		j["type"] = ControlTypeName(control.type);

		if (not control.text.empty())
			j["text"] = ToNarrow(control.text);

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

		if (!control.enabled)
			j["enabled"] = false;

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
			j["tooltip"] = ToNarrow(control.tooltip);

		if (!control.items.empty())
		{
			auto arr = nlohmann::json::array();
			for (auto& item : control.items)
				arr.push_back(ToNarrow(item));
			j["items"] = arr;
		}

		if (control.selectedIndex >= 0)
			j["selectedIndex"] = control.selectedIndex;

		if (control.value != 0)
			j["value"] = control.value;

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
			j["imagePath"] = ToNarrow(control.imagePath);

		if (!control.bindField.empty())
			j["bindField"] = control.bindField;

		if (!control.tabStop)
			j["tabStop"] = false;

		if (control.groupStart)
			j["groupStart"] = true;

		if (!control.accessibleName.empty())
			j["accessibleName"] = ToNarrow(control.accessibleName);

		if (!control.accessibleDescription.empty())
			j["accessibleDescription"] = ToNarrow(control.accessibleDescription);

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
				fj["family"] = ToNarrow(control.font.family);
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

		j["title"] = ToNarrow(form.title);
		j["width"] = form.width;
		j["height"] = form.height;

		if (form.style != Win32::Styles::OverlappedWindow)
			j["style"] = form.style;

		if (form.exStyle != 0)
			j["exStyle"] = form.exStyle;

		if (form.backgroundColor != -1)
			j["backgroundColor"] = ColorRefToHex(form.backgroundColor);

		if (!form.visible)
			j["visible"] = false;

		if (!form.enabled)
			j["enabled"] = false;

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

	// Saves a Form definition to a JSON file on disk (nothrow overload).
	auto TrySaveFormToFile(const Form& form, const std::filesystem::path& path) -> std::expected<void, FormException>
	{
		auto file = std::ofstream{ path };
		if (not file.is_open())
			return std::unexpected(FormException(FormErrorCode::FileWriteError,
				std::format("Cannot open file for writing: '{}'", path.string())));

		file << SerializeForm(form);
		return {};
	}

	// Saves a Form definition to a JSON file on disk (throwing overload).
	auto SaveFormToFile(const Form& form, const std::filesystem::path& path) -> void
	{
		auto result = TrySaveFormToFile(form, path);
		if (!result)
			throw std::move(result.error());
	}
}
