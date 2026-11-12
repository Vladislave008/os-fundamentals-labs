cmake_minimum_required(VERSION 3.15)

project(lab4 C)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "lab4 requires Linux")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(os_api STATIC src/os_api_posix.c)
target_include_directories(os_api PUBLIC src)
target_compile_definitions(os_api PRIVATE _GNU_SOURCE)

add_executable(parent src/app/parent.c)
target_compile_definitions(parent PRIVATE _POSIX_C_SOURCE=200809L)
target_link_libraries(parent PRIVATE os_api)

add_executable(child src/app/child.c)
target_link_libraries(child PRIVATE os_api)
add_dependencies(parent child)
