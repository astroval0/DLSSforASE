// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <d3d11.h>
#include "engine_types.h"

struct DlssEvaluateInputs {
    ID3D11Resource* Color = nullptr;
    ID3D11Resource* Depth = nullptr;
    ID3D11Resource* Motion = nullptr;
    ID3D11Resource* Output = nullptr;

    FIntRect Rect = {};

    float JitterX = 0.0f;
    float JitterY = 0.0f;
};

bool InitDlss(ID3D11Device* device);
bool VerifyDlssFeature(ID3D11DeviceContext* context, int width, int height);
bool EvaluateDlss(ID3D11DeviceContext* context, const DlssEvaluateInputs& inputs);