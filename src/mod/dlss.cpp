// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "dlss.h"

#include <filesystem>
#include <nvsdk_ngx.h>
#include<nvsdk_ngx_helpers_d3d.h>
#include <spdlog/spdlog.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace {

    // https://github.com/NVIDIA/DLSS/blob/9981c6350dc58c0ce21f024fd65e31e851c31689/include/nvsdk_ngx_defs.h#L464
    constexpr char ProjectId[] = "41737472-6f56-616c-3065-724204b777fa";

    NVSDK_NGX_Parameter* GParameters = nullptr;

    NVSDK_NGX_Handle* GFeature = nullptr;
    int GFeatureWidth = 0;
    int GFeatureHeight = 0;

    void NVSDK_CONV NgxLog(const char* message, NVSDK_NGX_Logging_Level, NVSDK_NGX_Feature) {
        std::string_view text = message;

        while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
            text.remove_suffix(1);
        }

        spdlog::debug("ngx: {}", text);
    }
}

bool InitDlss(ID3D11Device* device) {

    wchar_t path[MAX_PATH];
    GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), path, MAX_PATH);
    std::wstring dllDirectory = std::filesystem::path(path).parent_path().wstring();

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

bool VerifyDlssFeature(ID3D11DeviceContext *context, int width, int height) {
    if (width == GFeatureWidth && height == GFeatureHeight) {
        return GFeature != nullptr;
    }

    if (GFeature) {
        NVSDK_NGX_D3D11_ReleaseFeature(GFeature);
        GFeature = nullptr;
    }

    //cache failed sizes
    GFeatureWidth = width;
    GFeatureHeight = height;

    NVSDK_NGX_DLSS_Create_Params create = {};

    create.Feature.InWidth = width;
    create.Feature.InHeight = height;
    create.Feature.InTargetWidth = width;
    create.Feature.InTargetHeight = height;
    create.Feature.InPerfQualityValue = NVSDK_NGX_PerfQuality_Value_DLAA;

    create.InFeatureCreateFlags = NVSDK_NGX_DLSS_Feature_Flags_IsHDR | NVSDK_NGX_DLSS_Feature_Flags_MVLowRes | NVSDK_NGX_DLSS_Feature_Flags_DepthInverted | NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
    NVSDK_NGX_Result result = NGX_D3D11_CREATE_DLSS_EXT(context, &GFeature, GParameters, &create);

    if (NVSDK_NGX_FAILED(result)) {
        spdlog::error("dlss: create feature {}x{} failed {:#x}", width, height, static_cast<unsigned>(result));

        GFeature = nullptr;

        return false;
    }

    spdlog::info("dlss: feature created {}x{} DLAA", width, height);

    return true;
}
