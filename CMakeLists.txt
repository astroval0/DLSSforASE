cmake_minimum_required(VERSION 4.1)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

set(VCPKG_TARGET_TRIPLET x64-windows-static)

project(DLSSforASE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)

add_compile_definitions(UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)

find_package(wil CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(minhook CONFIG REQUIRED)

# shaders r compiled at build time, we should keep d3dcompiler out of our dll
file(GLOB FXC_DIRS "C:/Program Files (x86)/Windows Kits/10/bin/10.*/x64")
list(SORT FXC_DIRS)
list(REVERSE FXC_DIRS)
find_program(FXC fxc REQUIRED HINTS "$ENV{WindowsSdkVerBinPath}x64" ${FXC_DIRS})

function(add_shader target hlsl entry profile variable)
    get_filename_component(name ${hlsl} NAME_WE)
    set(header ${CMAKE_BINARY_DIR}/generated/${name}.h)
    add_custom_command(OUTPUT ${header} COMMAND ${FXC} /nologo /O3 /T ${profile} /E ${entry} /Vn ${variable} /Fh ${header} ${CMAKE_SOURCE_DIR}/${hlsl} MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/${hlsl})
    target_sources(${target} PRIVATE ${header})
endfunction()

#dlss stuffs (https://github.com/nvidia/dlss)
set(DLSS_SDK ${CMAKE_SOURCE_DIR}/third_party/DLSS)
add_library(ngx STATIC IMPORTED)
set_target_properties(ngx PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"

        IMPORTED_LOCATION_DEBUG
        "${DLSS_SDK}/lib/Windows_x86_64/x64/nvsdk_ngx_s_dbg.lib"

        IMPORTED_LOCATION_RELEASE
        "${DLSS_SDK}/lib/Windows_x86_64/x64/nvsdk_ngx_s.lib"

        MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE

        INTERFACE_INCLUDE_DIRECTORIES
        "${DLSS_SDK}/include"
)


configure_file(cmake/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h)
include_directories(${CMAKE_BINARY_DIR}/generated src/common)

file(GLOB_RECURSE LAUNCHER_SOURCES CONFIGURE_DEPENDS src/launcher/*.cpp)
add_executable(DLSSforASE WIN32 ${LAUNCHER_SOURCES})

target_link_libraries(DLSSforASE PRIVATE WIL::WIL spdlog::spdlog)

# dll side of things
file(GLOB_RECURSE MOD_SOURCES CONFIGURE_DEPENDS src/mod/*.cpp)
add_library(DLSSforASE_Mod SHARED ${MOD_SOURCES})
set_target_properties(DLSSforASE_Mod PROPERTIES OUTPUT_NAME dlss_for_ase)
target_include_directories(DLSSforASE_Mod PRIVATE src/mod)
target_link_libraries(DLSSforASE_Mod PRIVATE WIL::WIL spdlog::spdlog minhook::minhook ngx d3d11)
add_shader(DLSSforASE_Mod src/mod/shaders/depth_copy.hlsl main cs_5_0 DepthCopyCS)
add_shader(DLSSforASE_Mod src/mod/shaders/fullscreen.hlsl main vs_5_0 FullscreenVS)
add_shader(DLSSforASE_Mod src/mod/shaders/write_color.hlsl main ps_5_0 WriteColorPS)
add_shader(DLSSforASE_Mod src/mod/shaders/motion_vectors.hlsl main cs_5_0 MotionVectorsCS)
add_custom_command(TARGET DLSSforASE_Mod POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLSS_SDK}/lib/Windows_x86_64/rel/nvngx_dlss.dll $<TARGET_FILE_DIR:DLSSforASE_Mod>)
add_dependencies(DLSSforASE DLSSforASE_Mod)