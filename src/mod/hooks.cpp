// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "hooks.h"
#include "offsets.h"

#include <MinHook.h>
#include <spdlog/spdlog.h>


bool CreateHook(const char *name, uintptr_t rva, void *detour, void **original) {
    MH_STATUS status = MH_CreateHook(AtRva<void>(rva), detour, original);

    spdlog::info("hook {}: {}", name, MH_StatusToString(status));

    return status == MH_OK;
}

bool InstallHooks() {
    if (MH_Initialize() != MH_OK) {
        return false;
    }

    if (HookViewSetup() && HookPostProcess() && MH_EnableHook(MH_ALL_HOOKS) == MH_OK) {
        return true;
    }

    MH_Uninitialize();

    return false;
}
