// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <cstdint>

bool CreateHook(const char* name, uintptr_t rva, void* detour, void** original);

template <typename Fn> bool CreateHook(const char* name, uintptr_t rva, Fn detour, Fn* original) {
    return CreateHook(name, rva, reinterpret_cast<void*>(detour), reinterpret_cast<void**>(original));
}

bool HookViewSetup();
bool InstallHooks();
