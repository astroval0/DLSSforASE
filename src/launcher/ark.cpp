// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "ark.h"

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <wil/resource.h>
#include <format>
#include <fstream>

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

std::wstring FindBattleye(DWORD arkPid) {
    if (FindProcess(L"ShooterGame_BE.exe")) {
        return L"ShooterGame_BE.exe is running";
    }

    return {};
}

uint32_t ReadArkBuild(DWORD arkPid) {
    wil::unique_handle process{OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, arkPid)};

    if (!process) {
        return 0;
    }

    wchar_t exePath[MAX_PATH];
    DWORD length = MAX_PATH;

    if (!QueryFullProcessImageNameW(process.get(), 0, exePath, &length)) {
        return 0;
    }

    std::ifstream exe(exePath, std::ios::binary);
    IMAGE_DOS_HEADER dos = {};
    IMAGE_NT_HEADERS64 nt = {};

    if (!exe.read(reinterpret_cast<char*>(&dos), sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    exe.seekg(dos.e_lfanew);

    if (!exe.read(reinterpret_cast<char*>(&nt), sizeof(nt)) || nt.Signature != IMAGE_NT_SIGNATURE) {
        return 0;
    }

    return nt.FileHeader.TimeDateStamp;
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
