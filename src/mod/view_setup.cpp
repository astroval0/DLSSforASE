// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <algorithm>
#include <spdlog/spdlog.h>

#include "engine_types.h"
#include "hooks.h"
#include "offsets.h"

namespace {
    using ViewSetupFn = void(FSceneRenderer* renderer, FRHICommandListImmediate& cmdList);

    struct ViewJitter {
        const FViewInfo* View = nullptr;

        // projection matrix offset, -1..1 across the view rect. pixel = normalized device coordinate * size / 2
        float NdcX = 0.0f;
        float NdcY = 0.0f;
    };

    ViewSetupFn* GViewSetupOriginal = nullptr;
    uint32_t GFrames = 0;

    ViewJitter GJitter[4];

    FViewInfo* ViewAt(FSceneRenderer* renderer, int index) {
        uint8_t* views = Field<uint8_t*>(renderer, offset::SceneRenderer_Views);

        return reinterpret_cast<FViewInfo*>(views + index * offset::ViewInfo_Size);
    }

    bool IsMainView(const FViewInfo* view) {
        return Field<bool>(view, offset::View_bIsGameView) && !Field<bool>(view, offset::View_bIsSceneCapture) && !Field<bool>(view, offset::View_bIsReflectionCapture) && Field<void*>(view, offset::View_ViewState) != nullptr;
    }

    void ViewSetupDetour(FSceneRenderer* renderer, FRHICommandListImmediate& cmdList) {
        int numViews = std::min(Field<int>(renderer, offset::SceneRenderer_Views + 8), 4);
        float before[4][2] = {};

        for (int i = 0; i < numViews; ++i) {
            FViewInfo* view = ViewAt(renderer, i);
            const FMatrix& proj = Field<FMatrix>(view, offset::View_ViewMatrices);

            before[i][0] = proj.M[2][0];
            before[i][1] = proj.M[2][1];

            if (IsMainView(view)) {
                // we need jittered projection for DLSS, thankfully they left some of that code in.
                Field<uint8_t>(view, offset::View_AntiAliasingMethod) = AAM_TemporalAA;
            }
        }

        GViewSetupOriginal(renderer, cmdList);

        bool logFrame = ++GFrames % 600 < 8;

        for (ViewJitter& jitter : GJitter) {
            jitter = {};
        }

        for (int i = 0; i < numViews; ++i) {
            FViewInfo* view = ViewAt(renderer, i);
            const FMatrix& proj = Field<FMatrix>(view, offset::View_ViewMatrices);
            const FIntRect& rect = Field<FIntRect>(view, offset::View_ViewRect);

            GJitter[i].View = view;
            GJitter[i].NdcX = proj.M[2][0] - before[i][0];
            GJitter[i].NdcY = proj.M[2][1] - before[i][1];

            if (logFrame) {
                spdlog::info("jitter f{} v{} ndc=({:.6f},{:.6f}) px=({:.3f},{:.3f}) engine=({:.3f},{:.3f}) {}x{}", GFrames, i, GJitter[i].NdcX, GJitter[i].NdcY, GJitter[i].NdcX * rect.Width() * 0.5f, -GJitter[i].NdcY * rect.Height() * 0.5f, Field<float>(view, offset::View_TemporalJitterPixels), Field<float>(view, offset::View_TemporalJitterPixels + 4), rect.Width(), rect.Height());
            }
        }
    }
}

bool HookViewSetup() {
    return CreateHook("PreVisibilityFrameSetup", rva::PreVisibilityFrameSetup, &ViewSetupDetour, &GViewSetupOriginal);
}
