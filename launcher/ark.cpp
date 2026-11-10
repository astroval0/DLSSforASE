// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "ark.h"

#include <windows.h>
#include <shellapi.h>

#include <format>

namespace {
    // 346110 is ARK, option/1 is entry 1 in its steam launch config which launches without battleye.
    // its also undocumented in steamworks so it could break at any point
    constexpr wchar_t ArkNoBattlEyeLaunchUri[] = L"steam://launch/346110/option/1";
}

bool LaunchArkWithoutBattlEye(std::wstring& error) {
    HINSTANCE result = ShellExecuteW(nullptr, L"open", ArkNoBattlEyeLaunchUri, nullptr, nullptr, SW_SHOWNORMAL);

    // not a real HINSTANCE, its an error code. anything above 32 is success cuz windows is shit
    INT_PTR code = reinterpret_cast<INT_PTR>(result);

    if (code <= 32) {
        error = std::format(L"couldnt ask steam to launch ark (error {}). is steam installed?", code);

        return false;
    }

    return true;
}
