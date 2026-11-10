cmake_minimum_required(VERSION 4.1)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(DLSSforASE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)

find_package(wil CONFIG REQUIRED)

configure_file(cmake/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h)
include_directories(${CMAKE_BINARY_DIR}/generated)

#todo make me recurse
add_executable(DLSSforASE WIN32 launcher/ark.cpp launcher/main.cpp)

target_link_libraries(DLSSforASE PRIVATE WIL::WIL)