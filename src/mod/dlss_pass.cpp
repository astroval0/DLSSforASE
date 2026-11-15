// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "dlss_pass.h"

#include <d3d11_1.h>
#include <spdlog/spdlog.h>
#include <depth_copy.h>
#include "rhi.h"
#include "scene_targets.h"

namespace {
    struct PassState {
        bool Tried = false;
        bool Ready = false;

        ID3D11Device* Device = nullptr;
        ID3D11DeviceContext* Context = nullptr;

        wil::com_ptr<ID3D11DeviceContext1> Context1;
        wil::com_ptr<ID3DDeviceContextState> OurState;
    };

    PassState GPass;
}

bool InitPass() {
    EngineRhi rhi = FindEngineRhi();

    if (!rhi.Device || !rhi.Context) {
        return false;
    }

    auto device1 = wil::try_com_query<ID3D11Device1>(rhi.Device);
    GPass.Context1 = wil::try_com_query<ID3D11DeviceContext1>(rhi.Context);

    if (!device1 || !GPass.Context1) {
        spdlog::error("dlss pass: no D3D11.1 interfaces");
        return false;
    }

    D3D_FEATURE_LEVEL level = rhi.Device->GetFeatureLevel();
    UINT flags = (rhi.Device->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) ? D3D11_1_CREATE_DEVICE_CONTEXT_STATE_SINGLETHREADED : 0;

    HRESULT hr = device1->CreateDeviceContextState(flags, &level, 1, D3D11_SDK_VERSION, __uuidof(ID3D11Device1), nullptr, GPass.OurState.put());

    if (FAILED(hr)) {
        spdlog::error("dlss pass: CreateDeviceContextState {:#x}", static_cast<unsigned>(hr));
        return false;
    }

    GPass.Device = rhi.Device;
    GPass.Context = rhi.Context;

    return true;
}

void RunDlssPass(const DlssPassInputs &inputs) {
    if (!GPass.Tried) {
        GPass.Tried = true;
        GPass.Ready = InitPass();
    }

    if (!GPass.Ready || !inputs.Depth) {
        return;
    }

    wil::com_ptr<ID3DDeviceContextState> engineState;
    GPass.Context1->SwapDeviceContextState(GPass.OurState.get(), engineState.put());

    // we dont want to wipe the engines bindings
    GPass.Context->ClearState();
    GPass.Context1->SwapDeviceContextState(engineState.get(), nullptr);
}