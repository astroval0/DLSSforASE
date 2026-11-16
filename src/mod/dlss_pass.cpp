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

        wil::com_ptr<ID3D11ComputeShader> DepthCopy;

        wil::com_ptr<ID3D11Texture2D> Depth;
        wil::com_ptr<ID3D11UnorderedAccessView> DepthUAV;
        D3D11_TEXTURE2D_DESC DepthDesc = {};
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

    hr = rhi.Device->CreateComputeShader(DepthCopyCS, sizeof(DepthCopyCS), nullptr, GPass.DepthCopy.put());

    if (FAILED(hr)) {
        spdlog::error("dlss pass: CreateComputeShader {:#x}", static_cast<unsigned>(hr));
        return false;
    }

    GPass.Device = rhi.Device;
    GPass.Context = rhi.Context;

    return true;
}

bool VerifyDepthTexture(ID3D11ShaderResourceView* source) {
    wil::com_ptr<ID3D11Resource> resource;
    source->GetResource(resource.put());

    D3D11_TEXTURE2D_DESC engine = DescribeTexture(resource.get());

    if (GPass.Depth && engine.Width == GPass.DepthDesc.Width && engine.Height == GPass.DepthDesc.Height) {
        return true;
    }

    GPass.Depth.reset();
    GPass.DepthUAV.reset();

    D3D11_TEXTURE2D_DESC depth = {};

    depth.Width = engine.Width;
    depth.Height = engine.Height;
    depth.MipLevels = 1;
    depth.ArraySize = 1;
    depth.Format = DXGI_FORMAT_R32_FLOAT;
    depth.SampleDesc.Count = 1;
    depth.Usage = D3D11_USAGE_DEFAULT;
    depth.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

    if (engine.Width == 0 || FAILED(GPass.Device->CreateTexture2D(&depth, nullptr, GPass.Depth.put())) || FAILED(GPass.Device->CreateUnorderedAccessView(GPass.Depth.get(), nullptr, GPass.DepthUAV.put()))) {
        spdlog::error("dlss pass: depth texture {}x{} failed", engine.Width, engine.Height);
        GPass.Depth.reset();
        return false;
    }

    GPass.DepthDesc = depth;
    spdlog::info("dlss pass: depth {}x{} copied from fmt {}", depth.Width, depth.Height, static_cast<int>(engine.Format));

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

    if (VerifyDepthTexture(inputs.Depth.get())) {
        ID3D11ShaderResourceView* srv = inputs.Depth.get();
        ID3D11UnorderedAccessView* uav = GPass.DepthUAV.get();

        GPass.Context->CSGetShader(GPass.DepthCopy.get(), nullptr, 0);
        GPass.Context->CSSetShaderResources(0, 1, &srv);
        GPass.Context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
        GPass.Context->Dispatch((GPass.DepthDesc.Width + 7) / 8, (GPass.DepthDesc.Height + 7) / 8, 1);
    }

    // we dont want to wipe the engines bindings
    GPass.Context->ClearState();
    GPass.Context1->SwapDeviceContextState(engineState.get(), nullptr);
}