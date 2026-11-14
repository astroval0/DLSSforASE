// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <windows.h>

// these r only valid for ShooterGame.exe header timestamp 0x6949AAAEu

namespace rva {
    constexpr uintptr_t PreVisibilityFrameSetup = 0x039186A0;
    constexpr uintptr_t GDynamicRHI = 0x04B8D268;
    constexpr uintptr_t PostProcessingProcess = 0x039A9B20;
    constexpr uintptr_t GSceneRenderTargets = 0x04BDAD90;
}

template <typename T> T* AtRva(uintptr_t rva) {
    //windows stinks so we have to convert HMODULE to uintptr_t then we can add our relative virtual address
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)) + rva);
}

namespace offset {
    // FD3D11DynamicRHI
    constexpr size_t D3D11RHI_IMContext = 0x90;
    constexpr size_t D3D11RHI_Device = 0x98;
    // FRHICommandListBase
    constexpr size_t RHICommandList_CommandLink = 0x10;
    constexpr size_t RHICommandList_NumCommands = 0x1C;
    // FViewInfo
    constexpr size_t ViewInfo_Size = 0x1C20;
    constexpr size_t View_ViewRect = 0x30;
    constexpr size_t View_UnscaledViewRect = 0x40;
    constexpr size_t View_ViewMatrices = 0x70;
    constexpr size_t View_TemporalJitterPixels = 0x660;
    constexpr size_t View_bIsGameView = 0x7E4;
    constexpr size_t View_bIsSceneCapture = 0x7E7;
    constexpr size_t View_bIsReflectionCapture = 0x808;
    constexpr size_t View_AntiAliasingMethod = 0xB40;
    constexpr size_t View_ViewState = 0xD30;
    // FSceneRenderer::Views, TArray<FViewInfo>: data ptr at +0, int count at +8
    constexpr size_t SceneRenderer_Views = 0x90;

    constexpr size_t SceneRenderTargets_SceneColor = 0x30;
}

template <typename T> T& Field(const void* object, size_t offset) {
    return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(object) + offset);
}