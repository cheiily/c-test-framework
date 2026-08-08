cmake_minimum_required(VERSION 4.3)
project(playground C)

set(CMAKE_C_STANDARD 23)

add_executable(playground
        main.c
        program/program.h
        program/program.c
)

add_executable(playground_tests
        tests/tests.c
        tests/tests_framework.h
        program/program.h
        program/program.c
)
target_link_libraries(playground_tests m)