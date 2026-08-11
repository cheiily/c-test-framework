cmake_minimum_required(VERSION 4.3)
project(riichi C)

set(CMAKE_C_STANDARD 23)

add_executable(riichi
        main.c
        program/program.h
        program/program.c
)

add_executable(riichi_tests
        tests/tests.c
        tests/riichi.h
        program/program.h
        program/program.c
)
target_link_libraries(riichi_tests m)