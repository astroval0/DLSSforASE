// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "ark.h"

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <wil/resource.h>
#include <format>

namespace {
    // 346110 is ARK, option/1 is entry 1 in its steam launch config which launches without battleye.
    // its also undocumented in steamworks so it could break at any point
    constexpr wchar_t ArkNoBattlEyeLaunchUri[] = L"steam://launch/346110/option/1";
}

DWORD FindProcess(const wchar_t* exeName) {
    wil::unique_tool_help_snapshot snapshot{CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)};

    if (!snapshot) {
        return 0;
    }

    PROCESSENTRY32W entry = {
        sizeof(entry)
    };

    for (BOOL ok = Process32FirstW(snapshot.get(), &entry); ok; ok = Process32NextW(snapshot.get(), &entry)) {
        if (_wcsicmp(entry.szExeFile, exeName) == 0) {
            return entry.th32ProcessID;
        }
    }

    return 0;
}

DWORD FindArkProcess() {
    return FindProcess(L"ShooterGame.exe");
}

bool LaunchArkWithoutBattlEye(std::wstring& error) {
    HINSTANCE result = ShellExecuteW(nullptr, L"open", ArkNoBattlEyeLaunchUri, nullptr, nullptr, SW_SHOWNORMAL);

    // not a real HINSTANCE, its an error code. anything above 32 is success cuz windows is shit
    auto code = reinterpret_cast<INT_PTR>(result);

    if (code <= 32) {
        error = std::format(L"couldnt ask steam to launch ark (error {}). is steam installed?", code);

        return false;
    }

    return true;
}
