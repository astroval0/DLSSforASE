// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "rhi.h"
#include "offsets.h"

#include <spdlog/spdlog.h>

EngineRhi FindEngineRhi() {
    uint8_t* rhi = *AtRva<uint8_t*>(rva::GDynamicRHI);

    if (!rhi) {
        spdlog::error("rhi: engine has no D3D11 device yet!");
        return {};
    }

    EngineRhi found;

    found.Device = Field<ID3D11Device*>(rhi, offset::D3D11RHI_Device);
    found.Context = Field<ID3D11DeviceContext*>(rhi, offset::D3D11RHI_IMContext);

    spdlog::info("rhi: device {} context {} feature level {:#x}", static_cast<void*>(found.Device), static_cast<void*>(found.Device), static_cast<unsigned>(found.Device->GetFeatureLevel()));

    return found;
}
