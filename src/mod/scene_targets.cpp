// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "scene_targets.h"
#include <wil/com.h>
#include "offsets.h"

SceneTargets FindSceneTargets() {
    const uint8_t* targets = AtRva<uint8_t>(rva::GSceneRenderTargets);
    int shadingPath = Field<int>(targets, offset::SceneRenderTargets_CurrentShadingPath);

    SceneTargets found;

    found.Color = Field<IPooledRenderTarget*>(targets, offset::SceneRenderTargets_SceneColor + 8 * (shadingPath & 1));
    found.Depth = Field<IPooledRenderTarget*>(targets, offset::SceneRenderTargets_SceneDepthZ);

    return found;
}

ID3D11Resource* NativeResource(const IPooledRenderTarget *pooled) {
    const void* texture = pooled ? Field<void*>(pooled, offset::PooledRT_TargetableTexture) : nullptr;

    return texture ? Field<ID3D11Resource*>(texture, offset::D3D11Texture_Resource) : nullptr;
}

D3D11_TEXTURE2D_DESC DescribeTexture(ID3D11Resource *resource) {
    D3D11_TEXTURE2D_DESC desc = {};

    if (resource) {
        if (auto texture = wil::try_com_query<ID3D11Texture2D>(resource)) {
            texture->GetDesc(&desc);
        }
    }

    return desc;
}
