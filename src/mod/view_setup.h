// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once
#include "engine_types.h"

struct ViewJitter {
    const FViewInfo* View = nullptr;

    // projection matrix offset, -1..1 across the view rect. pixel = normalized device coordinate * size / 2
    float NdcX = 0.0f;
    float NdcY = 0.0f;
};

bool IsMainView(const FViewInfo* view);

const ViewJitter* FindJitter(const FViewInfo* view);