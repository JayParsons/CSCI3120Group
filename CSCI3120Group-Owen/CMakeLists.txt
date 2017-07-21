cmake_minimum_required(VERSION 3.7)
project(CSCI3120Group_Owen)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.c
        network.c
        network.h
        Priority_Heap.c
        Priority_Heap.h
        sws.c)

add_executable(CSCI3120Group_Owen ${SOURCE_FILES})