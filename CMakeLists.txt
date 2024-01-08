cmake_minimum_required(VERSION 3.18)
project(Langton C)

set(CMAKE_C_STANDARD 11)

add_executable(Langton main.c
        world.h
        ant.h
        ant.c
        world.c)

target_link_libraries(Langton pthread ws2_32)
