// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "ark.h"

#include <windows.h>
#include <wil/registry.h>
#include <vdf_parser.hpp>

namespace {
    bool HasArkExe(const std::filesystem::path& dir) {
        return !dir.empty() && std::filesystem::is_regular_file(dir / L"ShooterGame\\Binaries\\Win64\\ShooterGame.exe");
    }

    std::filesystem::path FindArkInSteamLibraries() {
        std::optional<std::wstring> steam = wil::reg::try_get_value_string(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath");

        if (!steam) {
            return {};
        }

        // vdf::read would throw if the file didnt open
        std::ifstream file(std::filesystem::path(*steam) / L"steamapps\\libraryfolders.vdf");

        if (!file) {
            return {};
        }

        std::error_code error;
        tyti::vdf::object libraries = tyti::vdf::read(file, error);

        if (error) {
            return {};
        }

        for (const auto& [index, library] : libraries.childs) {
            auto apps = library->childs.find("apps");

            if (apps == library->childs.end() || !apps->second->attribs.contains("346110")) {
                continue;
            }

            const std::string& path = library->attribs["path"];

            return std::filesystem::path(std::u8string(path.begin(), path.end())) / L"steamapps\\common\\ARK";
        }

        return {};
    }
}

std::filesystem::path FindArkInstall() {
    if (std::optional<std::wstring> uninstall = wil::reg::try_get_value_string(HKEY_LOCAL_MACHINE, L"Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 346110", L"InstallLocation")) {
        return *uninstall;
    }

    std::filesystem::path arkDir = FindArkInSteamLibraries();

    if (HasArkExe(arkDir)) {
        return arkDir;
    }

    return {};
}
