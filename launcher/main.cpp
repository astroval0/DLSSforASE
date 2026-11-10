// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "ark.h"
#include "version.h"
#include <windows.h>
#include <string>

namespace {

    constexpr int IdLaunch = 101;

    HFONT GFont = nullptr;

    HWND AddControl(HWND parent, const wchar_t* type, const wchar_t* text, DWORD style, int x, int y, int width, int height, int id) {
        HWND control = CreateWindowW(type, text, WS_CHILD | WS_VISIBLE | style, x, y, width, height, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);

        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(GFont), TRUE);
        return control;
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

                HWND launch = AddControl(window, L"BUTTON", L"Launch ARK (no BattlEye)", BS_PUSHBUTTON, 16, 104, 200, 32, IdLaunch);

                EnableWindow(launch, true);

                return 0;
            }
            case WM_COMMAND: {
                if (LOWORD(wParam) == IdLaunch) {
                    OnLaunch(window);
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
    windowClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    windowClass.lpszClassName = L"DLSSforASELauncher";

    RegisterClassW(&windowClass);

    HWND window = CreateWindowW(windowClass.lpszClassName, L"DLSSforASE v" DLSSFORASE_VERSION_W,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
        470, 270, nullptr, nullptr, instance, nullptr);

    ShowWindow(window, showCommand);

    MSG message;

    while (GetMessageW(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return 0;
}