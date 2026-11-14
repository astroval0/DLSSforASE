// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <spdlog/spdlog.h>

#include "engine_types.h"
#include "engine_types.h"
#include "hooks.h"
#include "offsets.h"
#include "scene_targets.h"
#include "view_setup.h"

namespace {
    //the last parameter is a TRefCountPtr<IPooledRenderTarget>& (which is just a ptr :p)
    using PostProcessFn = void(FPostProcessing* postProcessing, FRHICommandListImmediate& cmdList, FViewInfo& view, IPooledRenderTarget*& velocity);

    PostProcessFn* GPostProcessOriginal = nullptr;
    uint32_t GFrames = 0;

    void PostProcessDetour(FPostProcessing* postProcessing, FRHICommandListImmediate& cmdList, FViewInfo& view, IPooledRenderTarget*& velocity) {
        if (IsMainView(&view) && ++GFrames % 600 == 0) {
            SceneTargets targets = FindSceneTargets();

            D3D11_TEXTURE2D_DESC color = DescribeTexture(NativeResource(targets.Color));
            D3D11_TEXTURE2D_DESC depth = DescribeTexture(NativeResource(targets.Depth));
            D3D11_TEXTURE2D_DESC motion = DescribeTexture(NativeResource(velocity));

            const FIntRect& rect = Field<FIntRect>(&view, offset::View_ViewRect);
            const FIntRect& unscaled = Field<FIntRect>(&view, offset::View_UnscaledViewRect);
            const ViewJitter* jitter = FindJitter(&view);

            spdlog::info("post f{} rect {}x{} unscaled {}x{} aa={} | color {} {}x{} fmt {} | depth {} fmt {} | velocity {} {}x{} fmt {} | jitter px=({:.3f},{:.3f})",
                GFrames, rect.Width(), rect.Height(), unscaled.Width(), unscaled.Height(), Field<uint8_t>(&view, offset::View_AntiAliasingMethod), static_cast<void*>(targets.Color),
                color.Width, color.Height, static_cast<int>(color.Format), static_cast<void*>(targets.Depth),
                static_cast<int>(depth.Format), static_cast<void*>(velocity), motion.Width, motion.Height,
                static_cast<int>(motion.Format), jitter ? jitter->NdcX * rect.Width() * 0.5f : 0.0f, jitter ? -jitter->NdcY * rect.Height() * 0.5f : 0.0f);
        }

        GPostProcessOriginal(postProcessing, cmdList, view, velocity);
    }
}

bool HookPostProcess() {
    return CreateHook("FPostProcessing::Process", rva::PostProcessingProcess, &PostProcessDetour, &GPostProcessOriginal);
}