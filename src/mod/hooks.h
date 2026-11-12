// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <cstdint>

bool CreateHook(const char* name, uintptr_t rva, void* detour, void** original);
bool InstallHooks();