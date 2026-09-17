cmake_minimum_required(VERSION 3.16)
project(kbe VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
)

target_compile_options(kbe PRIVATE -Wall -Wextra)