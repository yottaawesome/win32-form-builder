module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Commdlg.h>

export module designer:fileops;
import std;
import formbuilder;
import :state;
import :helpers;
import :canvas;

namespace Designer
{

auto ShowSaveDialog(HWND owner, std::filesystem::path& outPath) -> bool
{
    wchar_t filename[MAX_PATH] = {};
    if (!outPath.empty())
        wcscpy_s(filename, outPath.wstring().c_str());

    OPENFILENAMEW ofn = {
        .lStructSize = sizeof(OPENFILENAMEW),
        .hwndOwner = owner,
        .lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0",
        .lpstrFile = filename,
        .nMaxFile = MAX_PATH,
        .Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
        .lpstrDefExt = L"json",
    };

    if (!GetSaveFileNameW(&ofn))
        return false;

    outPath = filename;
    return true;
}

auto ShowOpenDialog(HWND owner, std::filesystem::path& outPath) -> bool
{
    wchar_t filename[MAX_PATH] = {};

    OPENFILENAMEW ofn = {
        .lStructSize = sizeof(OPENFILENAMEW),
        .hwndOwner = owner,
        .lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0",
        .lpstrFile = filename,
        .nMaxFile = MAX_PATH,
        .Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
    };

    if (!GetOpenFileNameW(&ofn))
        return false;

    outPath = filename;
    return true;
}

export auto PromptSaveIfDirty(DesignState& state) -> bool
{
    if (!state.dirty)
        return true;

    auto result = MessageBoxW(state.surfaceHwnd,
        L"Save changes before continuing?",
        L"Form Designer",
        MB_YESNOCANCEL | MB_ICONQUESTION);

    if (result == IDCANCEL)
        return false;

    if (result == IDYES)
    {
        if (state.currentFile.empty())
        {
            if (!ShowSaveDialog(state.surfaceHwnd, state.currentFile))
                return false;
        }
        FormDesigner::SaveFormToFile(state.form, state.currentFile);
        state.dirty = false;
        UpdateTitle(state);
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
    FormDesigner::SaveFormToFile(state.form, state.currentFile);
    state.dirty = false;
    UpdateTitle(state);
}

export void DoSaveAs(DesignState& state)
{
    auto path = state.currentFile;
    if (!ShowSaveDialog(state.surfaceHwnd, path))
        return;
    state.currentFile = path;
    FormDesigner::SaveFormToFile(state.form, state.currentFile);
    state.dirty = false;
    UpdateTitle(state);
}

export void DoOpen(DesignState& state)
{
    if (!PromptSaveIfDirty(state))
        return;

    auto path = std::filesystem::path{};
    if (!ShowOpenDialog(state.surfaceHwnd, path))
        return;

    try
    {
        state.form = FormDesigner::LoadFormFromFile(path);
        state.currentFile = path;
        state.dirty = false;
        RebuildControls(state);
        UpdateTitle(state);
    }
    catch (const std::exception& ex)
    {
        auto msg = std::string{ "Failed to open file:\n" } + ex.what();
        auto wide = std::wstring(msg.begin(), msg.end());
        MessageBoxW(state.surfaceHwnd, wide.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
}

export void DoNew(DesignState& state)
{
    if (!PromptSaveIfDirty(state))
        return;

    state.form = FormDesigner::Form{};
    state.currentFile.clear();
    state.dirty = false;
    RebuildControls(state);
    UpdateTitle(state);
}

}
