cmake_minimum_required(VERSION 3.16)
project(kbe VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(kbe 
    src/main.c
    src/gb.c
    src/gb.h
    src/bus.c
    src/bus.h
    src/cart.c
    src/cart.h
    src/cpu.c
    src/cpu.h
    src/io.h
    src/ppu.h
    src/ppu.c
)

target_compile_options(kbe PRIVATE -Wall -Wextra)
target_link_libraries(kbe PRIVATE raylib)