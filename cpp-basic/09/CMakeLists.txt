cmake_minimum_required(VERSION 3.7.2)
project(crc32_crack CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(topk_words main.cpp)

if (MSVC)
    target_compile_options(topk_words PUBLIC /W4)
else ()
    target_compile_options(topk_words PUBLIC -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif ()
