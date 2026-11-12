// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include <iterator>
#include <spdlog/spdlog.h>

#include "hooks.h"
#include "offsets.h"
#include "rhi_command.h"

namespace {
    //todo: find the real types ya dickhead
    using ViewSetupFn = void (*)(void* renderer, void* cmdlist);

    ViewSetupFn GViewSetupOriginal = nullptr;
    uint32_t GFrames = 0;

    void ViewSetupDetour(void* renderer, void* cmdlist) {
        GViewSetupOriginal(renderer, cmdlist);

        if (++GFrames % 600 == 0) {
            spdlog::info("{} frames", GFrames);

            LoadRhiWork(cmdlist, [frame = GFrames] {
                spdlog::info("rhi: work for frame {} ran", frame);
            });
        }
    }
}

bool HookViewSetup() {
    // i could prolly do a typed wrapper / template to avoid reinterpret_casting but icba
    return CreateHook("PreVisibilityFrameSetup", rva::PreVisibilityFrameSetup, reinterpret_cast<void*>(&ViewSetupDetour), reinterpret_cast<void**>(&GViewSetupOriginal));
}