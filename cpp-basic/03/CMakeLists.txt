cmake_minimum_required(VERSION 3.5)

project(03.homework)

add_executable(guess_the_number
    guess_the_number.cpp
    arguments.cpp
    randval.cpp
    storage.cpp
)

set_target_properties(guess_the_number PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(guess_the_number PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wold-style-cast
    -Wdouble-promotion
)
