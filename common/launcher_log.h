// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

constexpr wchar_t LauncherWindowClass[] = L"DLSSforASELauncher";

class LauncherSink : public spdlog::sinks::base_sink<std::mutex> {
    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override {
            HWND launcher = FindWindowW(LauncherWindowClass, nullptr);

            if (!launcher) {
                return;
            }

            spdlog::memory_buf_t formatted;

            formatter_->format(msg, formatted);

            COPYDATASTRUCT data = {0, static_cast<DWORD>(formatted.size()), formatted.data()};

            SendMessageTimeoutW(launcher, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&data), SMTO_ABORTIFHUNG, 100, nullptr);
        }

        void flush_() override {

        }
};

inline void UseLauncherLog(const char* name) {
    auto logger = std::make_shared<spdlog::logger>(name, std::make_shared<LauncherSink>());
    logger->set_pattern("[%H:%M:%S.%e %t] [%n] [%l] %v");

    spdlog::set_default_logger(logger);
}