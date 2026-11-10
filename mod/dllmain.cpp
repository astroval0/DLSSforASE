// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <windows.h>

namespace {
    DWORD WINAPI Worker(LPVOID) {
        MessageBoxW(nullptr, L"Hello from inside ARK", L"Astro's DLSS for ASE", MB_OK | MB_TOPMOST | MB_SETFOREGROUND);

        return 0;
    }

    BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
        if (reason == DLL_PROCESS_ATTACH) {
            DisableThreadLibraryCalls(module);

            HANDLE thread = CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr);

            if (thread) {
                CloseHandle(thread);
            }

            return TRUE;
        }
    }
}