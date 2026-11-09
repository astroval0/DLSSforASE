cmake_minimum_required(VERSION 4.1)
project(astrosdlssforase VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)

add_executable(astrosdlssforase WIN32 launcher/main.cpp)
