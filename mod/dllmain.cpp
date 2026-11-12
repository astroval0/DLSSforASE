// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <version.h>
#include <windows.h>
#include <spdlog/spdlog.h>
#include "launcher_log.h"
#include <filesystem>

namespace {
    DWORD WINAPI Worker(LPVOID) {
        UseLauncherLog("mod");
        spdlog::info("DLSSforASE {} loaded into pid {}", DLSSFORASE_VERSION, GetCurrentProcessId());

        return 0;
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {

        HANDLE thread = CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr);

        if (thread) {
            CloseHandle(thread);
        }
    }

    return TRUE;
}