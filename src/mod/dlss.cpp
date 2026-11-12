// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "dlss.h"
#include <nvsdk_ngx.h>
#include <spdlog/spdlog.h>

namespace {

    // https://github.com/NVIDIA/DLSS/blob/9981c6350dc58c0ce21f024fd65e31e851c31689/include/nvsdk_ngx_defs.h#L464
    constexpr char ProjectId[] = "41737472-6f56-616c-3065-724204b777fa";

    NVSDK_NGX_Parameter* GParameters = nullptr;

    void NVSDK_CONV NgxLog(const char* message, NVSDK_NGX_Logging_Level, NVSDK_NGX_Feature) {
        std::string_view text = message;

        while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
            text.remove_suffix(1);
        }

        spdlog::debug("ngx: {}", text);
    }
}

bool InitDlss(ID3D11Device *device, const std::wstring &dllDirectory) {
    const wchar_t* paths[] = {dllDirectory.c_str()};

    NVSDK_NGX_FeatureCommonInfo info = {};

    info.PathListInfo.Path = paths;
    info.PathListInfo.Length = 1;
    info.LoggingInfo.LoggingCallback = NgxLog;
    info.LoggingInfo.MinimumLoggingLevel = NVSDK_NGX_LOGGING_LEVEL_ON;
    info.LoggingInfo.DisableOtherLoggingSinks = true;

    NVSDK_NGX_Result result = NVSDK_NGX_D3D11_Init_with_ProjectID(ProjectId, NVSDK_NGX_ENGINE_TYPE_CUSTOM, "1.0", dllDirectory.c_str(), device, &info);

    if (NVSDK_NGX_FAILED(result)) {
        spdlog::error("dlss: NGX init failed {:#x}", static_cast<unsigned>(result));
        return false;
    }

    result = NVSDK_NGX_D3D11_GetCapabilityParameters(&GParameters);

    if (NVSDK_NGX_FAILED(result) || !GParameters) {
        spdlog::error("dlss: capability query failed {:#x}", static_cast<unsigned>(result));
        return false;
    }

    int available = 0;
    int needsDriver = 0;

    NVSDK_NGX_Parameter_GetI(GParameters, NVSDK_NGX_Parameter_SuperSampling_Available, &available);
    NVSDK_NGX_Parameter_GetI(GParameters, NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, &needsDriver);

    spdlog::info("dlss: available {} needs newer driver {}", available, needsDriver);

    return available != 0;
}
