// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <d3d11.h>
#include  "engine_types.h"
#include "scene_targets.h"

struct SceneTargets {
    IPooledRenderTarget* Color = nullptr;
    IPooledRenderTarget* Depth = nullptr;
};

SceneTargets FindSceneTargets();

ID3D11Resource* NativeResource(const IPooledRenderTarget* pooled);
D3D11_TEXTURE2D_DESC DescribeTexture(ID3D11Resource* resource);