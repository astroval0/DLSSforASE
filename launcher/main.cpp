// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <format>
#include <future>

#include "ark.h"
#include "inject.h"
#include "version.h"
#include <windows.h>
#include <string>
#include <spdlog/spdlog.h>

#include "launcher_log.h"

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {
    constexpr int IdLaunch = 101;
    constexpr int IdInject = 102;

    constexpr UINT_PTR StatusTimer = 1;

    constexpr wchar_t ModDllName[] = L"dlss_for_ase.dll";

    HFONT GFont = nullptr;

    HWND GLogPane = nullptr;
    HWND GLaunchButton = nullptr;
    HWND GInjectButton = nullptr;

    std::wstring ExeDirectory() {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring dir = path;
        return dir.substr(0, dir.find_last_of(L'\\'));
    }

    std::wstring WhyNotInject(DWORD arkPid) {
        std::wstring battleye = FindBattleye(arkPid);
        if (!battleye.empty()) {
            return L"BattlEye is running (" + battleye + L"). Close ARK and use \"Launch ARK (no BattlEye)\".";
        }
        uint32_t build = ReadArkBuild(arkPid);

        if (build != DLSSFORASE_ARK_BUILD) {
            return std::format(
                L"this ARK build ({:08X}) isnt the one the mod was made for ({:08X}). ARK probably updated.", build,
                DLSSFORASE_ARK_BUILD);
        }

        return {};
    }

    void AppendLogLine(const char* utf8, size_t bytes) {
        std::wstring text(bytes, L'\0');
        int length = MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(bytes), text.data(), static_cast<int>(bytes));
        text.resize(length);

        int end = GetWindowTextLengthW(GLogPane);

        SendMessageW(GLogPane, EM_SETSEL, end, end);
        SendMessageW(GLogPane, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));
        SendMessageW(GLogPane, EM_SCROLLCARET, 0, 0);
    }

    HWND AddControl(HWND parent, const wchar_t *type, const wchar_t *text, DWORD style, int x, int y, int width,
                    int height, int id) {
        HWND control = CreateWindowW(type, text, WS_CHILD | WS_VISIBLE | style, x, y, width, height, parent,
                                     reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);

        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(GFont), TRUE);
        return control;
    }

    void RefreshStatus() {
        DWORD pid = FindArkProcess();

        bool loaded = pid && IsModuleLoaded(pid, ModDllName);

        static DWORD knownPid = 0;
        static bool knownLoaded = false;

        if (pid != knownPid) {
            if (pid) {
                spdlog::info("ARK is running (pid {})", pid);
            }
            else {
                spdlog::info("ARK closed");
            }
        }

        if (loaded && !knownLoaded) {
            spdlog::info("mod is loaded");
        }

        knownPid = pid;
        knownLoaded = loaded;

        EnableWindow(GLaunchButton, !pid);
        EnableWindow(GInjectButton, pid && !loaded);
    }

    void OnInject(HWND window) {
        DWORD pid = FindArkProcess();

        if (!pid) {
            return;
        }

        std::wstring problem = WhyNotInject(pid);
        std::wstring error;

        if (problem.empty() && InjectDll(pid, ExeDirectory() + L"\\" + ModDllName, error)) {
            RefreshStatus();

            return;
        }

        MessageBoxW(window, (L"not injecting.\n\n" + (problem.empty() ? error : problem)).c_str(),
                    L"Astro's DLSSforASE", MB_ICONWARNING);
    }

    void OnLaunch(HWND window) {
        std::wstring error;

        if (!LaunchArkWithoutBattlEye(error)) {
            MessageBoxW(window, error.c_str(), L"Astro's DLSSforASE", MB_ICONERROR);
        }
    }

    LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_CREATE: {
                NONCLIENTMETRICSW metrics = {
                    sizeof(metrics)
                };

                SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
                GFont = CreateFontIndirectW(&metrics.lfMessageFont);

                GLogPane = AddControl(window, L"EDIT", L"", ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL, 16, 12, 420, 200, 0);

                GLaunchButton = AddControl(window, L"BUTTON", L"Launch ARK (no BattlEye)", BS_PUSHBUTTON, 16, 224, 200,32, IdLaunch);
                GInjectButton = AddControl(window, L"BUTTON", L"Inject", BS_PUSHBUTTON, 226, 224, 100, 32, IdInject);

                UseLauncherLog("launcher");
                spdlog::info("DLSSforASE {} by AstroVal0 :D", DLSSFORASE_VERSION);

                SetTimer(window, StatusTimer, 1000, nullptr);
                RefreshStatus();

                return 0;
            }
            case WM_TIMER: {
                RefreshStatus();
                return 0;
            }
            case WM_COPYDATA: {
                auto* data = reinterpret_cast<COPYDATASTRUCT*>(lParam);

                AppendLogLine(static_cast<const char*>(data->lpData), data->cbData);

                return TRUE;
            }
            case WM_COMMAND: {
                if (LOWORD(wParam) == IdLaunch) {
                    OnLaunch(window);
                } else if (LOWORD(wParam) == IdInject) {
                    OnInject(window);
                }

                return 0;
            }
            case WM_DESTROY: {
                PostQuitMessage(0);
                return 0;
            }
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    WNDCLASSW windowClass = {};

    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    windowClass.lpszClassName = LauncherWindowClass;

    RegisterClassW(&windowClass);

    HWND window = CreateWindowW(windowClass.lpszClassName, L"Astro's DLSS for ASE v" DLSSFORASE_VERSION_W,
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
                                470, 320,
                                nullptr, nullptr, instance, nullptr);

    ShowWindow(window, showCommand);

    MSG message;

    while (GetMessageW(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return 0;
}
