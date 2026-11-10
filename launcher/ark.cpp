// Astro's DLSS for ASE - https://github.com/astroval0/astros-dlss-for-ase
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "ark.h"

#include <windows.h>
#include <wil/registry.h>

std::filesystem::path FindArkInstall() {
    std::optional<std::wstring> uninstall = wil::reg::try_get_value_string(HKEY_LOCAL_MACHINE, L"Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 346110", L"InstallLocation");

    if (uninstall) {
        return *uninstall;
    }

    return {};
}
