cmake_minimum_required(VERSION 3.31)
project(ALLO)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE ALLO_SRC
    Allo/*.h
    Allo/*.cpp
)

add_executable(ALLO main.cpp ${ALLO_SRC})
