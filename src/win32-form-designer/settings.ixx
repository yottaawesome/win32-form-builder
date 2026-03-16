export module designer:settings;
import std;
import formbuilder;
import :win32;
import :state;

namespace Designer
{

	export auto GetSettingsPath() -> std::filesystem::path
	{
		wchar_t exePath[Win32::MaxPath] = {};
		Win32::GetModuleFileNameW(nullptr, exePath, Win32::MaxPath);
		auto p = std::filesystem::path(exePath).parent_path() / L"designer.ini";
		return p;
	}

	// Loads all settings as a key=value map from designer.ini.
	auto LoadSettingsMap() -> std::map<std::string, std::string>
	{
		auto result = std::map<std::string, std::string>{};
		auto path = GetSettingsPath();
		auto in = std::ifstream(path);
		if (!in) return result;

		auto line = std::string{};
		while (std::getline(in, line))
		{
			auto eq = line.find('=');
			if (eq != std::string::npos)
				result[line.substr(0, eq)] = line.substr(eq + 1);
		}
		return result;
	}

	// Saves all settings as key=value pairs to designer.ini.
	void SaveSettingsMap(const std::map<std::string, std::string>& settings)
	{
		auto path = GetSettingsPath();
		auto out = std::ofstream(path);
		if (!out) return;
		for (auto& [key, value] : settings)
			out << key << "=" << value << "\n";
	}

	export void SaveThemePreference(bool isDark)
	{
		auto settings = LoadSettingsMap();
		settings["theme"] = isDark ? "dark" : "light";
		SaveSettingsMap(settings);
	}

	export auto LoadThemePreference() -> bool
	{
		auto settings = LoadSettingsMap();
		auto it = settings.find("theme");
		if (it != settings.end())
			return it->second == "dark";
		return false;
	}

	// Prepends a path to the recent files list, removes duplicates, caps at MAX_RECENT_FILES.
	export void AddRecentFile(std::vector<std::filesystem::path>& recentFiles,
		const std::filesystem::path& filePath)
	{
		auto canonical = std::filesystem::weakly_canonical(filePath);
		std::erase_if(recentFiles, [&](const std::filesystem::path& p) {
			return std::filesystem::weakly_canonical(p) == canonical;
		});
		recentFiles.insert(recentFiles.begin(), canonical);
		if (static_cast<int>(recentFiles.size()) > MAX_RECENT_FILES)
			recentFiles.resize(MAX_RECENT_FILES);
	}

	export void SaveRecentFiles(const std::vector<std::filesystem::path>& recentFiles)
	{
		auto settings = LoadSettingsMap();
		// Remove old recent entries.
		for (int i = 1; i <= MAX_RECENT_FILES; ++i)
			settings.erase("recent" + std::to_string(i));
		// Write current list.
		for (int i = 0; i < static_cast<int>(recentFiles.size()); ++i)
			settings["recent" + std::to_string(i + 1)] = recentFiles[i].string();
		SaveSettingsMap(settings);
	}

	export auto LoadRecentFiles() -> std::vector<std::filesystem::path>
	{
		auto settings = LoadSettingsMap();
		auto result = std::vector<std::filesystem::path>{};
		for (int i = 1; i <= MAX_RECENT_FILES; ++i)
		{
			auto it = settings.find("recent" + std::to_string(i));
			if (it == settings.end() || it->second.empty())
				break;
			result.emplace_back(it->second);
		}
		return result;
	}

	// Rebuilds the Recent Files submenu in the File menu.
	export void RebuildRecentFilesMenu(Win32::HWND surfaceHwnd,
		const std::vector<std::filesystem::path>& recentFiles)
	{
		auto menuBar = Win32::GetMenu(surfaceHwnd);
		if (!menuBar) return;

		// File menu is position 0 in the menu bar.
		auto fileMenu = Win32::GetSubMenu(menuBar, 0);
		if (!fileMenu) return;

		// Remove any existing recent file items and the separator before them.
		// Recent items use IDs IDM_FILE_RECENT_BASE + 0..9.
		for (int i = 0; i < MAX_RECENT_FILES; ++i)
			Win32::DeleteMenu(fileMenu, IDM_FILE_RECENT_BASE + i, Win32::Menu::ByCommand);
		// Remove the separator that we insert (tagged with IDM_FILE_RECENT_BASE + MAX_RECENT_FILES).
		Win32::DeleteMenu(fileMenu, IDM_FILE_RECENT_BASE + MAX_RECENT_FILES, Win32::Menu::ByCommand);

		if (recentFiles.empty()) return;

		// Find the position of IDM_FILE_OPEN to insert after it.
		int insertPos = -1;
		int count = Win32::GetMenuItemCount(fileMenu);
		for (int i = 0; i < count; ++i)
		{
			auto id = static_cast<Win32::UINT>(Win32::GetMenuItemID(fileMenu, i));
			if (id == IDM_FILE_OPEN)
			{
				insertPos = i + 1;
				break;
			}
		}
		if (insertPos < 0) return;

		// Insert a separator, then each recent file entry.
		Win32::InsertMenuW(fileMenu, insertPos, Win32::Menu::ByPosition | Win32::Menu::Separator,
			IDM_FILE_RECENT_BASE + MAX_RECENT_FILES, nullptr);
		++insertPos;

		for (int i = 0; i < static_cast<int>(recentFiles.size()); ++i)
		{
			auto label = std::to_wstring(i + 1) + L"  " + recentFiles[i].filename().wstring();
			Win32::InsertMenuW(fileMenu, insertPos + i,
				Win32::Menu::ByPosition | Win32::Menu::String,
				IDM_FILE_RECENT_BASE + i, label.c_str());
		}
	}

}
