// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.
#pragma once

#include <cstdint>
#include <string>
#include <windows.h>

bool LaunchArkWithoutBattlEye(std::wstring& error);

// returns 0 if nothing found
DWORD FindArkProcess();

std::wstring FindBattleye(DWORD arkPid);

uint32_t ReadArkBuild(DWORD arkPid);