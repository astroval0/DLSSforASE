// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <d3d11.h>
#include <wil/com.h>

#include "engine_types.h"

struct DlssPassInputs {
    wil::com_ptr<ID3D11ShaderResourceView> Depth;
    FIntRect Rect = {};
    bool Probe = false;
};

// RHI thread onlyy
void RunDlssPass(const DlssPassInputs& inputs);