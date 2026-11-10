// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.
#pragma once
#include <string>
#include <windows.h>

bool InjectDll(DWORD pid, const std::wstring& dllPath, std::wstring& error);
bool IsModuleLoaded(DWORD pid, const wchar_t* moduleName);
