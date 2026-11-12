// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once
#include <cstdint>
#include <windows.h>

template <typename T> T* AtRva(uintptr_t rva) {
    //windows stinks so we have to convert HMODULE to uintptr_t then we can add our relative virtual address
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)) + rva);
}