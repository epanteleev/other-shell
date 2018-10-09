cmake_minimum_required(VERSION 3.10)
project(other_shell)

set(CMAKE_C_STANDARD 99)

add_definitions("-Wall ")
include_directories("/editor")

set(SOURCES signal_handler.c
            signal_handler.h
            common.h)

add_executable(other_shell main.c  ${SOURCES})
