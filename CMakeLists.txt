cmake_minimum_required (VERSION 3.1)
project (MIMUW-PW-AUTOMATON)

set(CMAKE_C_FLAGS "-g -Wall")

macro (add_executable _name)
    # invoke built-in add_executable
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} err rt m)
    endif()
endmacro()

set(SOURCE_FILES
    src/sort.c
    )

add_library(err src/err.c)

add_executable(sort ${SOURCE_FILES})
