cmake_minimum_required(VERSION 3.15)

project(lab3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT (UNIX AND NOT APPLE))
    message(FATAL_ERROR "lab3 is a POSIX/Linux lab (pthreads), current platform is not supported")
endif()

message(STATUS "Building for Linux (POSIX threads)...")

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)

add_library(filter STATIC
    src/matrix_filter.h
    src/matrix_filter.c
    src/qsort.h
    src/qsort.c
)
target_include_directories(filter PUBLIC src)
target_compile_definitions(filter PUBLIC _POSIX_C_SOURCE=200809L)

add_library(core STATIC
    src/matrix_io.h
    src/matrix_io.c
    src/util.h
    src/util.c
    src/pool.h
    src/pool.c
    src/bench.h
    src/bench.c
)
target_link_libraries(core PUBLIC filter Threads::Threads)

add_executable(lab3 src/app/main.c)
target_link_libraries(lab3 PRIVATE core Threads::Threads)
