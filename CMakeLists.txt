cmake_minimum_required(VERSION 3.10)
project(other_shell)

set(CMAKE_C_STANDARD 11)

add_definitions("-Wall ")

set(SOURCES signal_handler.c
            signal_handler.h
            parser.c
            parser.h
            process_list.c
            process_list.h
            execute.c
            execute.h
            common.h)

add_executable(other_shell main.c  ${SOURCES})
target_link_libraries(other_shell)
