// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once
#include <cstdint>

//real layouts never need to exist here
class FSceneRenderer;
class FRHICommandListImmediate;
class FViewInfo;
class FPostProcessing;
class IPooledRenderTarget;

constexpr uint8_t AAM_TemporalAA = 2;

struct FIntRect {
    int MinX, MinY, MaxX, MaxY;

    int Width() const { return MaxX - MinX; }
    int Height() const { return MaxY - MinY; }
};

struct FMatrix {
    float M[4][4];
};
