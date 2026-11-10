// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "ark.h"
#include "version.h"
#include <windows.h>
#include <string>

namespace {
    LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_CREATE: {
                NONCLIENTMETRICSW metrics = {
                    sizeof(metrics)
                };

                SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
                HFONT font = CreateFontIndirectW(&metrics.lfMessageFont);

                std::filesystem::path arkDir = FindArkInstall();
                std::wstring status = arkDir.empty() ? L"cannot find ark. do you have it installed through steam?" : L"ARK: " + arkDir.native();

                HWND label = CreateWindowW(L"STATIC", status.c_str(), WS_CHILD | WS_VISIBLE, 16, 12, 420, 84, window, nullptr, nullptr, nullptr);
                SendMessageW(label, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

                return 0;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
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