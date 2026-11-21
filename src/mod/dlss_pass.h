// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wil/com.h>

#include "engine_types.h"

struct DlssPassInputs {
    wil::com_ptr<ID3D11ShaderResourceView> Depth;
    wil::com_ptr<ID3D11ShaderResourceView> Velocity;

    FIntRect Rect = {};

    float JitterX = 0.0f;
    float JitterY = 0.0f;

    DirectX::XMFLOAT4X4 ClipToPrevClip = {};

    bool Probe = false;

    wil::com_ptr<ID3D11Resource> Color;
};

// RHI thread onlyy
void RunDlssPass(const DlssPassInputs& inputs);