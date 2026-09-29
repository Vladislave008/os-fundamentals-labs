cmake_minimum_required(VERSION 3.15)

project(lab2 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/W4 /WX-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(os_api STATIC src/os_api.h)
target_include_directories(os_api PUBLIC src)

if(WIN32)
    message(STATUS "Building for Windows (Win32 API)...")
    target_sources(os_api PRIVATE src/os_api_win.c)
    target_link_libraries(os_api PRIVATE kernel32)
elseif(UNIX AND NOT APPLE)
    message(STATUS "Building for Linux (POSIX API)...")
    target_sources(os_api PRIVATE src/os_api_posix.c)
elseif(APPLE)
    message(FATAL_ERROR "macOS is not supported yet")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

add_executable(parent src/app/parent.c)
target_link_libraries(parent PRIVATE os_api)

add_executable(child src/app/child.c)
target_link_libraries(child PRIVATE os_api)