cmake_minimum_required(VERSION 3.27)
project(c C)

set(CMAKE_C_STANDARD 11)

add_executable(c
        main.c
        intArray.h
        stringUtils.h
        memAlloc.h
        allocators/array.h
)
