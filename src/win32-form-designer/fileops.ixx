export module designer:fileops;
import std;
import formbuilder;
import :state;
import :settings;
import :helpers;
import :canvas;

namespace Designer
{

	auto ShowSaveDialog(Win32::HWND owner, std::filesystem::path& outPath) -> bool
	{
		wchar_t filename[Win32::MaxPath] = {};
		if (!outPath.empty())
		{
			auto str = outPath.wstring();
			auto n = std::min(str.size(), static_cast<std::size_t>(Win32::MaxPath - 1));
			std::copy_n(str.data(), n, filename);
		}

		Win32::OPENFILENAMEW ofn = {
			.lStructSize = sizeof(Win32::OPENFILENAMEW),
			.hwndOwner = owner,
			.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0",
			.lpstrFile = filename,
			.nMaxFile = Win32::MaxPath,
			.Flags = Win32::FileDialog::OverwritePrompt | Win32::FileDialog::PathMustExist,
			.lpstrDefExt = L"json",
		};

		if (!Win32::GetSaveFileNameW(&ofn))
			return false;

		outPath = filename;
		return true;
	}

	auto ShowOpenDialog(Win32::HWND owner, std::filesystem::path& outPath) -> bool
	{
		wchar_t filename[Win32::MaxPath] = {};

		Win32::OPENFILENAMEW ofn = {
			.lStructSize = sizeof(Win32::OPENFILENAMEW),
			.hwndOwner = owner,
			.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0",
			.lpstrFile = filename,
			.nMaxFile = Win32::MaxPath,
			.Flags = Win32::FileDialog::FileMustExist | Win32::FileDialog::PathMustExist,
		};

		if (!Win32::GetOpenFileNameW(&ofn))
			return false;

		outPath = filename;
		return true;
	}

	export auto PromptSaveIfDirty(DesignState& state) -> bool
	{
		if (!state.dirty)
			return true;

		auto result = Win32::MessageBoxW(state.surfaceHwnd,
			L"Save changes before continuing?",
			L"Form Designer",
			Win32::Mb_YesNoCancel | Win32::Mb_IconQuestion);

		if (result == Win32::Id_Cancel)
			return false;

		if (result == Win32::Id_Yes)
		{
			if (state.currentFile.empty())
			{
				if (!ShowSaveDialog(state.surfaceHwnd, state.currentFile))
					return false;
			}
			SyncGuidesToForm(state);
			FormDesigner::SaveFormToFile(state.form, state.currentFile);
			state.dirty = false;
			UpdateTitle(state);
			UpdateStatusBar(state);
		}

		return true;
	}

	export void DoSave(DesignState& state)
	{
		if (state.currentFile.empty())
		{
			if (!ShowSaveDialog(state.surfaceHwnd, state.currentFile))
				return;
		}
		SyncGuidesToForm(state);
		FormDesigner::SaveFormToFile(state.form, state.currentFile);
		state.dirty = false;
		AddRecentFile(state.recentFiles, state.currentFile);
		SaveRecentFiles(state.recentFiles);
		RebuildRecentFilesMenu(state.surfaceHwnd, state.recentFiles);
		UpdateTitle(state);
		UpdateStatusBar(state);
	}

	export void DoSaveAs(DesignState& state)
	{
		auto path = state.currentFile;
		if (!ShowSaveDialog(state.surfaceHwnd, path))
			return;
		state.currentFile = path;
		SyncGuidesToForm(state);
		FormDesigner::SaveFormToFile(state.form, state.currentFile);
		state.dirty = false;
		AddRecentFile(state.recentFiles, state.currentFile);
		SaveRecentFiles(state.recentFiles);
		RebuildRecentFilesMenu(state.surfaceHwnd, state.recentFiles);
		UpdateTitle(state);
		UpdateStatusBar(state);
	}

	// Core open logic — loads a form from an already-known path (no dialog).
	export void DoOpenFile(DesignState& state, const std::filesystem::path& path)
	{
		try
		{
			PushUndo(state);
			state.form = FormDesigner::LoadFormFromFile(path);
			state.currentFile = path;
			state.dirty = false;
			RebuildControls(state);
			SyncNextGroupId(state);
			AddRecentFile(state.recentFiles, path);
			SaveRecentFiles(state.recentFiles);
			RebuildRecentFilesMenu(state.surfaceHwnd, state.recentFiles);
			UpdateTitle(state);
			UpdateStatusBar(state);
		}
		catch (const std::exception& ex)
		{
			auto msg = std::string{ "Failed to open file:\n" } + ex.what();
			auto wide = std::wstring(msg.begin(), msg.end());
			Win32::MessageBoxW(state.surfaceHwnd, wide.c_str(), L"Error",
				Win32::Mb_Ok | Win32::Mb_IconError);
		}
	}

	export void DoOpen(DesignState& state)
	{
		if (!PromptSaveIfDirty(state))
			return;

		auto path = std::filesystem::path{};
		if (!ShowOpenDialog(state.surfaceHwnd, path))
			return;

		DoOpenFile(state, path);
	}

	export void DoNew(DesignState& state)
	{
		if (!PromptSaveIfDirty(state))
			return;

		PushUndo(state);
		state.form = FormDesigner::Form{};
		state.currentFile.clear();
		state.dirty = false;
		RebuildControls(state);
		UpdateTitle(state);
		UpdateStatusBar(state);
	}

	auto ShowCppSaveDialog(Win32::HWND owner, std::filesystem::path& outPath) -> bool
	{
		wchar_t filename[Win32::MaxPath] = {};
		if (!outPath.empty())
		{
			auto str = outPath.wstring();
			auto n = std::min(str.size(), static_cast<std::size_t>(Win32::MaxPath - 1));
			std::copy_n(str.data(), n, filename);
		}

		Win32::OPENFILENAMEW ofn = {
			.lStructSize = sizeof(Win32::OPENFILENAMEW),
			.hwndOwner = owner,
			.lpstrFilter = L"C++ Files (*.cpp)\0*.cpp\0All Files (*.*)\0*.*\0",
			.lpstrFile = filename,
			.nMaxFile = Win32::MaxPath,
			.Flags = Win32::FileDialog::OverwritePrompt | Win32::FileDialog::PathMustExist,
			.lpstrDefExt = L"cpp",
		};

		if (!Win32::GetSaveFileNameW(&ofn))
			return false;

		outPath = filename;
		return true;
	}

	export void DoExportCpp(DesignState& state)
	{
		// Ask Classic vs Modules.
		auto result = Win32::MessageBoxW(state.surfaceHwnd,
			L"Use C++20 modules style?\n\n"
			L"Yes = import std; (requires C++20 module support)\n"
			L"No  = Classic #include style (broader compatibility)",
			L"Export to C++",
			Win32::Mb_YesNoCancel | Win32::Mb_IconQuestion);

		if (result == Win32::Id_Cancel)
			return;

		auto useModules = (result == Win32::Id_Yes);

		// Choose output path.
		auto path = std::filesystem::path{};
		if (!ShowCppSaveDialog(state.surfaceHwnd, path))
			return;

		// Generate and write.
		auto code = FormDesigner::GenerateCode(state.form, useModules);

		auto file = std::ofstream{ path };
		if (!file.is_open())
		{
			Win32::MessageBoxW(state.surfaceHwnd, L"Failed to write file.",
				L"Export Error", Win32::Mb_Ok | Win32::Mb_IconError);
			return;
		}
		file << code;
		file.close();

		Win32::MessageBoxW(state.surfaceHwnd,
			L"C++ source exported successfully.",
			L"Export to C++", Win32::Mb_Ok | Win32::Mb_IconInformation);
	}

	auto ShowRcSaveDialog(Win32::HWND owner, std::filesystem::path& outPath) -> bool
	{
		wchar_t filename[Win32::MaxPath] = {};
		if (!outPath.empty())
		{
			auto str = outPath.wstring();
			auto n = std::min(str.size(), static_cast<std::size_t>(Win32::MaxPath - 1));
			std::copy_n(str.data(), n, filename);
		}

		Win32::OPENFILENAMEW ofn = {
			.lStructSize = sizeof(Win32::OPENFILENAMEW),
			.hwndOwner = owner,
			.lpstrFilter = L"Resource Files (*.rc)\0*.rc\0All Files (*.*)\0*.*\0",
			.lpstrFile = filename,
			.nMaxFile = Win32::MaxPath,
			.Flags = Win32::FileDialog::OverwritePrompt | Win32::FileDialog::PathMustExist,
			.lpstrDefExt = L"rc",
		};

		if (!Win32::GetSaveFileNameW(&ofn))
			return false;

		outPath = filename;
		return true;
	}

	export void DoExportRc(DesignState& state)
	{
		auto path = std::filesystem::path{};
		if (!ShowRcSaveDialog(state.surfaceHwnd, path))
			return;

		// Generate RC dialog content.
		auto rcContent = FormDesigner::GenerateRcDialog(state.form);
		auto headerContent = FormDesigner::GenerateRcHeader(state.form);

		// Write .rc file.
		auto rcFile = std::ofstream{ path };
		if (!rcFile.is_open())
		{
			Win32::MessageBoxW(state.surfaceHwnd, L"Failed to write .rc file.",
				L"Export Error", Win32::Mb_Ok | Win32::Mb_IconError);
			return;
		}
		rcFile << rcContent;
		rcFile.close();

		// Write resource.h alongside the .rc file.
		auto headerPath = path.parent_path() / "resource.h";
		auto hdrFile = std::ofstream{ headerPath };
		if (!hdrFile.is_open())
		{
			Win32::MessageBoxW(state.surfaceHwnd, L"RC file written but failed to write resource.h.",
				L"Export Warning", Win32::Mb_Ok | Win32::Mb_IconWarning);
			return;
		}
		hdrFile << headerContent;
		hdrFile.close();

		Win32::MessageBoxW(state.surfaceHwnd,
			L"RC dialog exported successfully.\n\nFiles written:\n- .rc dialog template\n- resource.h header",
			L"Export to RC", Win32::Mb_Ok | Win32::Mb_IconInformation);
	}

}
