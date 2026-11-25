// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <DirectXMath.h>
#include <spdlog/spdlog.h>

#include "dlss_pass.h"
#include "engine_types.h"
#include "hooks.h"
#include "offsets.h"
#include "rhi_command.h"
#include "scene_targets.h"
#include "view_setup.h"

namespace {
    //the last parameter is a TRefCountPtr<IPooledRenderTarget>& (which is just a ptr :p)
    using PostProcessFn = void(FPostProcessing* postProcessing, FRHICommandListImmediate& cmdList, FViewInfo& view, IPooledRenderTarget*& velocity);

    PostProcessFn* GPostProcessOriginal = nullptr;
    uint32_t GFrames = 0;
    bool GLastReset = false;

    struct LastCamera {
        bool Valid = false;
        const void* ViewState = nullptr;
        DirectX::XMFLOAT4X4 TranslatedViewProjection = {};
        DirectX::XMFLOAT3 PreViewTranslation = {};
        float JitterNdcX = 0.0f;
        float JitterNdcY = 0.0f;
    };

    LastCamera GLastCamera;

    // teleports and fast travel dont always flag a camera cut, 20m in one frame is a jump not movement
    constexpr float JumpDistance = 2000.0f;

    float CameraMoveDistance(const FViewInfo& view) {
        using namespace DirectX;

        const XMFLOAT3& pvt = Field<XMFLOAT3>(&view, offset::View_ViewMatrices + offset::ViewMatrices_PreViewTranslation);

        return XMVectorGetX(XMVector3Length(XMLoadFloat3(&pvt) - XMLoadFloat3(&GLastCamera.PreViewTranslation)));
    }

    bool ShouldResetHistory(const FViewInfo& view) {
        return !GLastCamera.Valid || Field<bool>(&view, offset::View_bCameraCut) || Field<void*>(&view, offset::View_ViewState) != GLastCamera.ViewState || CameraMoveDistance(view) > JumpDistance;
    }

    DirectX::XMFLOAT4X4 MakeClipToPrevClip(const FViewInfo& view, const ViewJitter* jitter, bool reset) {
        using namespace DirectX;

        XMFLOAT4X4 result;
        XMStoreFloat4x4(&result, XMMatrixIdentity());

        if (reset) {
            return result;
        }

        const XMFLOAT4X4& invTvp = Field<XMFLOAT4X4>(&view, offset::View_ViewMatrices + offset::ViewMatrices_InvTranslatedViewProjection);
        const XMFLOAT3& pvt = Field<XMFLOAT3>(&view, offset::View_ViewMatrices + offset::ViewMatrices_PreViewTranslation);
        const XMFLOAT3& lastPvt = GLastCamera.PreViewTranslation;

        float jitterX = jitter ? jitter->NdcX : 0.0f;
        float jitterY = jitter ? jitter->NdcY : 0.0f;

        XMMATRIX cameraMove = XMMatrixTranslation(lastPvt.x - pvt.x, lastPvt.y - pvt.y, lastPvt.z - pvt.z);

        // jitter change is baked in cuz dlss wants motion without jitter
        XMMATRIX jitterChange = XMMatrixTranslation(jitterX - GLastCamera.JitterNdcX, jitterY - GLastCamera.JitterNdcY, 0.0f);

        XMStoreFloat4x4(&result, XMLoadFloat4x4(&invTvp) * cameraMove * XMLoadFloat4x4(&GLastCamera.TranslatedViewProjection) * jitterChange);

        return result;
    }

    void RememberCamera(const FViewInfo& view, const ViewJitter* jitter) {
        GLastCamera.Valid = true;
        GLastCamera.ViewState = Field<void*>(&view, offset::View_ViewState);
        GLastCamera.TranslatedViewProjection = Field<DirectX::XMFLOAT4X4>(&view, offset::View_ViewMatrices + offset::ViewMatrices_TranslatedViewProjection);
        GLastCamera.PreViewTranslation = Field<DirectX::XMFLOAT3>(&view, offset::View_ViewMatrices + offset::ViewMatrices_PreViewTranslation);
        GLastCamera.JitterNdcX = jitter ? jitter->NdcX : 0.0f;
        GLastCamera.JitterNdcY = jitter ? jitter->NdcY : 0.0f;
    }

    void PostProcessDetour(FPostProcessing* postProcessing, FRHICommandListImmediate& cmdList, FViewInfo& view, IPooledRenderTarget*& velocity) {
        if (IsMainView(&view)) {
            SceneTargets targets = FindSceneTargets();
            const FIntRect& rect = Field<FIntRect>(&view, offset::View_ViewRect);
            const ViewJitter* jitter = FindJitter(&view);

            DlssPassInputs inputs;

            inputs.Color = NativeResource(targets.Color);
            inputs.Depth = NativeSRV(targets.Depth);
            inputs.Velocity = NativeSRV(velocity);
            inputs.Rect = rect;
            inputs.JitterX = jitter ? jitter->NdcX * rect.Width() * 0.5f : 0.0f;
            inputs.JitterY = jitter ? -jitter->NdcY * rect.Height() * 0.5f : 0.0f;
            inputs.Reset = ShouldResetHistory(view);
            inputs.ClipToPrevClip = MakeClipToPrevClip(view, jitter, inputs.Reset);
            inputs.Probe = ++GFrames % 600 == 0;

            // only log the first frame so a stuck camera cut cant spam the launcher
            if (inputs.Reset && !GLastReset) {
                spdlog::info("post f{} dlss history reset, camera cut {} moved {:.0f}", GFrames, Field<bool>(&view, offset::View_bCameraCut), CameraMoveDistance(view));
            }

            GLastReset = inputs.Reset;

            RememberCamera(view, jitter);

            if (inputs.Color && inputs.Depth) {
                LoadRhiWork(&cmdList, [inputs] { RunDlssPass(inputs); });

                // the forced TemporalAA just turns into fxaa in post, only drop it while dlss is really doing the aa
                if (IsDlssPassWorking()) {
                    Field<uint8_t>(&view, offset::View_AntiAliasingMethod) = AAM_None;
                }
            }

            if (inputs.Probe) {
                D3D11_TEXTURE2D_DESC color = DescribeTexture(NativeResource(targets.Color));
                D3D11_TEXTURE2D_DESC depth = DescribeTexture(NativeResource(targets.Depth));
                D3D11_TEXTURE2D_DESC motion = DescribeTexture(NativeResource(velocity));

                const FIntRect& unscaled = Field<FIntRect>(&view, offset::View_UnscaledViewRect);

                spdlog::info("post f{} rect {}x{} unscaled {}x{} aa={} | color {} {}x{} fmt {} | depth {} fmt {} | velocity {} {}x{} fmt {} | jitter px=({:.3f},{:.3f})",
                    GFrames, rect.Width(), rect.Height(), unscaled.Width(), unscaled.Height(), Field<uint8_t>(&view, offset::View_AntiAliasingMethod), static_cast<void*>(targets.Color),
                    color.Width, color.Height, static_cast<int>(color.Format), static_cast<void*>(targets.Depth),
                    static_cast<int>(depth.Format), static_cast<void*>(velocity), motion.Width, motion.Height,
                    static_cast<int>(motion.Format), inputs.JitterX, inputs.JitterY);
            }
        }

        GPostProcessOriginal(postProcessing, cmdList, view, velocity);
    }
}

bool HookPostProcess() {
    return CreateHook("FPostProcessing::Process", rva::PostProcessingProcess, &PostProcessDetour, &GPostProcessOriginal);
}