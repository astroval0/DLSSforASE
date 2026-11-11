// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <version.h>
#include <windows.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>

namespace {

    HMODULE GModule = nullptr;

    void OpenLog() {
        wchar_t dllPath[MAX_PATH];

        GetModuleFileNameW(GModule, dllPath, MAX_PATH);

        try {
            auto logger = spdlog::basic_logger_mt("mod", std::filesystem::path(dllPath).replace_extension(L".log").string());
            logger->set_pattern("[%H:%M:%S.%e %t] [%l] %v");

            logger->flush_on(spdlog::level::trace);
            spdlog::set_default_logger(logger);
        } catch (const spdlog::spdlog_ex&) {

        }
    }

    DWORD WINAPI Worker(LPVOID) {

        OpenLog();

        spdlog::info("DLSSforASE {} loaded into pid {}", DLSSFORASE_VERSION, GetCurrentProcessId());

        return 0;
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {

        GModule = module;

        HANDLE thread = CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr);

        if (thread) {
            CloseHandle(thread);
        }
    }

    return TRUE;
}