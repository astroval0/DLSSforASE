// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <windows.h>

// these r only valid for ShooterGame.exe header timestamp 0x6949AAAEu

namespace rva {
    constexpr uintptr_t PreVisibilityFrameSetup = 0x039186A0;
    constexpr uintptr_t GDynamicRHI = 0x04B8D268;
}

template <typename T> T* AtRva(uintptr_t rva) {
    //windows stinks so we have to convert HMODULE to uintptr_t then we can add our relative virtual address
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)) + rva);
}

namespace offset {
    // FD3D11DynamicRHI
    constexpr size_t D3D11RHI_IMContext = 0x90;
    constexpr size_t D3D11RHI_Device = 0x98;
}

template <typename T> T& Field(const void* object, size_t offset) {
    // add our offset to the object ptr
    return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(object) + offset);
}