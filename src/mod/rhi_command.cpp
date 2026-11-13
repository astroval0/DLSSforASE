// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "rhi_command.h"

#include "offsets.h"

namespace {
    //copied from FRHICommandBase in build 0x6949AAAEu (tho no vtable)
    struct RhiCommand {
        RhiCommand* Next = nullptr;
        void (*Execute)(void* cmdList, RhiCommand* self) = nullptr;
        std::function<void()> Work;
    };

    // the executor reads Next before calling this, so deleting self here is bueno
    void ExecuteRhiCommand(void*, RhiCommand* self) {
        self->Work();
        delete self;
    }
}

//copied from FRHICommandListBase::AllocCommand in build 0x6949AAAEu (tho keep memory so it survives a list reset)
void LoadRhiWork(void *cmdList, std::function<void()> work) {
    auto* command = new RhiCommand;
    command->Execute = &ExecuteRhiCommand;
    command->Work = std::move(work);

    auto& link = Field<RhiCommand**>(cmdList, offset::RHICommandList_CommandLink);

    *link = command;
    link = &command->Next;

    ++Field<uint32_t>(cmdList, offset::RHICommandList_NumCommands);
}
