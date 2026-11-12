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

#todo make me recurse
add_executable(DLSSforASE WIN32 src/launcher/ark.cpp src/launcher/main.cpp src/launcher/inject.cpp)

target_link_libraries(DLSSforASE PRIVATE WIL::WIL spdlog::spdlog)

# dll side of things
add_library(DLSSforASE_Mod SHARED src/mod/dllmain.cpp
        src/mod/hooks.cpp
        src/mod/view_setup.cpp
        src/mod/rhi.cpp
        src/mod/dlss.cpp
        src/mod/rhi_command.cpp
)
set_target_properties(DLSSforASE_Mod PROPERTIES OUTPUT_NAME dlss_for_ase)
target_include_directories(DLSSforASE_Mod PRIVATE src/mod)
target_link_libraries(DLSSforASE_Mod PRIVATE spdlog::spdlog minhook::minhook ngx d3d11)
add_custom_command(TARGET DLSSforASE_Mod POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLSS_SDK}/lib/Windows_x86_64/rel/nvngx_dlss.dll $<TARGET_FILE_DIR:DLSSforASE_Mod>)
add_dependencies(DLSSforASE DLSSforASE_Mod)