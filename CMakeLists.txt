cmake_minimum_required(VERSION 3.10)
project(other_shell)

set(CMAKE_C_STANDARD 11)

add_definitions("-Wall ")

include_directories(readline/)
#add_subdirectory(readline)
set(SOURCES parser.c
            parser.h
            process_list.c
            process_list.h
            common.h
            execute.c
            execute.h)

add_executable(other_shell main.c  ${SOURCES})
target_link_libraries(other_shell -lreadline )
