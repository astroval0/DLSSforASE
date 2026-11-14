// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "scene_targets.h"

#include "offsets.h"

SceneTargets FindSceneTargets() {
    const uint8_t* targets = AtRva<uint8_t>(rva::GSceneRenderTargets);
}
