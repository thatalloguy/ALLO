cmake_minimum_required(VERSION 3.31)
project(AlloLanguage C)

set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE ALLO_SRC
        Allo/*.h
        Allo/*.c
)

add_executable(AlloLanguage main.c ${ALLO_SRC})
