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

configure_file(cmake/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h)
include_directories(${CMAKE_BINARY_DIR}/generated src/common)

#todo make me recurse
add_executable(DLSSforASE WIN32 src/launcher/ark.cpp src/launcher/main.cpp src/launcher/inject.cpp)

target_link_libraries(DLSSforASE PRIVATE WIL::WIL spdlog::spdlog)

# dll side of things
add_library(DLSSforASE_Mod SHARED src/mod/dllmain.cpp
        src/mod/hooks.cpp
        src/mod/view_setup.cpp)
set_target_properties(DLSSforASE_Mod PROPERTIES OUTPUT_NAME dlss_for_ase)
target_include_directories(DLSSforASE_Mod PRIVATE src/mod)
target_link_libraries(DLSSforASE_Mod PRIVATE spdlog::spdlog minhook::minhook)
add_dependencies(DLSSforASE DLSSforASE_Mod)