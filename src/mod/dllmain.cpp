// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <filesystem>
#include <version.h>
#include <windows.h>
#include <spdlog/spdlog.h>

#include "dlss.h"
#include "launcher_log.h"

#include "hooks.h"
#include "rhi.h"

namespace {
    DWORD WINAPI Worker(LPVOID module) {
        UseLauncherLog("mod");
        spdlog::info("DLSSforASE {} loaded into pid {}", DLSSFORASE_VERSION, GetCurrentProcessId());

        if (!InstallHooks()) {
            spdlog::error("hooking failed :(");
        }

        wchar_t path[MAX_PATH];
        GetModuleFileNameW(static_cast<HMODULE>(module), path, MAX_PATH);

        EngineRhi rhi = FindEngineRhi();

        if (rhi.Device && !InitDlss(rhi.Device, std::filesystem::path(path).parent_path().wstring())) {
            spdlog::error("DLSS is not available :(");
        }

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