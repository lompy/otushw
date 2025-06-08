cmake_minimum_required(VERSION 3.5)

project(06-07.homework)

include(FetchContent)
FetchContent_Declare(GTEST
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 6910c9d)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(GTEST)

add_executable(containers
    src/main.cpp
)
target_include_directories(
    containers PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
set_target_properties(containers PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_compile_options(containers PRIVATE
    -g
    -O0
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

add_executable(test
    test/main_tests.cpp
    test/sutovec_tests.cpp
    test/sutolist_tests.cpp
)
target_include_directories(
    test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_libraries(test PRIVATE gtest)
set_target_properties(test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_compile_options(test PRIVATE
    -g
    -O0
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
