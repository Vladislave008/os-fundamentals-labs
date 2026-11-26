cmake_minimum_required(VERSION 3.15)
project(lab5 C)

if(WIN32)
    set(loader_source src/loader_windows.c)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(loader_source src/loader_linux.c)
else()
    message(FATAL_ERROR "lab5 requires Linux or Windows")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(impl1 SHARED src/impl1.c)
add_library(impl2 SHARED src/impl2.c)

add_library(command_loop STATIC src/command_loop.c)
target_include_directories(command_loop PUBLIC src)

add_executable(program1 src/app/linked.c)
target_link_libraries(program1 PRIVATE command_loop impl1)

add_executable(program2 src/app/runtime.c ${loader_source})
target_link_libraries(program2 PRIVATE command_loop ${CMAKE_DL_LIBS})
target_compile_definitions(program2 PRIVATE
    "IMPL1_PATH=\"./$<TARGET_FILE_NAME:impl1>\""
    "IMPL2_PATH=\"./$<TARGET_FILE_NAME:impl2>\"")
add_dependencies(program2 impl1 impl2)
