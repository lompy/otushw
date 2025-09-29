cmake_minimum_required(VERSION 3.15)
# Enable compilation database for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
  squedl
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check for std::expected availability
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
  #include <expected>
  int main() { std::expected<int, int> e{0}; return 0; }
" HAS_STD_EXPECTED)

if(NOT HAS_STD_EXPECTED)
  include(FetchContent)
  FetchContent_Declare(
    expected-lite
    GIT_REPOSITORY https://github.com/martinmoene/expected-lite.git
    GIT_TAG v0.6.3
  )
  FetchContent_MakeAvailable(expected-lite)
endif()

function(set_strict_warnings target_name)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target_name} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Werror
      -Wold-style-cast
      -Wnon-virtual-dtor
      -Woverloaded-virtual
    )
  endif()

  if(MSVC)
    target_compile_options(${target_name} PRIVATE
      /W4
      /WX
      /permissive-
    )
  endif()
endfunction()

option(SQUEDL_BUILD_TESTS "Build tests" ON)
option(SQUEDL_ENABLE_CLANG_TIDY "Enable clang-tidy checks during build" ON)
option(SQUEDL_ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(SQUEDL_ENABLE_TSAN "Enable Thread Sanitizer" OFF)

# Sanitizer setup
if(SQUEDL_ENABLE_ASAN AND SQUEDL_ENABLE_TSAN)
  message(FATAL_ERROR "Cannot enable both Address Sanitizer and Thread Sanitizer at the same time")
endif()

if(SQUEDL_ENABLE_ASAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
  else()
    message(FATAL_ERROR "Address Sanitizer is only supported with GCC and Clang")
  endif()
endif()

if(SQUEDL_ENABLE_TSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
  else()
    message(FATAL_ERROR "Thread Sanitizer is only supported with GCC and Clang")
  endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(${PROJECT_NAME}
  src/squedl.cpp
)

set_strict_warnings(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/squedl/detail
)

if(NOT HAS_STD_EXPECTED)
  target_link_libraries(${PROJECT_NAME} PUBLIC nonstd::expected-lite)
  target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    $<TARGET_PROPERTY:nonstd::expected-lite,INTERFACE_INCLUDE_DIRECTORIES>
  )
endif()

if(SQUEDL_ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  if(CLANG_TIDY_EXE)
    add_custom_target(
      clang-tidy
      COMMAND ${CLANG_TIDY_EXE}
        --warnings-as-errors=*
        --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
        --extra-arg=-I${CMAKE_SOURCE_DIR}/include
        --extra-arg=-I${expected-lite_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/include/squedl/*.hpp
        ${CMAKE_SOURCE_DIR}/include/squedl/detail/*.hpp
        -- -isystem ${CMAKE_SOURCE_DIR}/build/_deps
      COMMENT "Running clang-tidy"
      DEPENDS ${PROJECT_NAME}
    )
  else()
    message(WARNING "clang-tidy not found!")
  endif()
endif()

if(SQUEDL_BUILD_TESTS)
  enable_testing()
  
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  
  FetchContent_MakeAvailable(googletest)
  
  add_subdirectory(test)
endif()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_BINARY_DIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install(EXPORT ${PROJECT_NAME}-targets
#   FILE ${PROJECT_NAME}-targets.cmake
#   NAMESPACE ${PROJECT_NAME}::
#   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
# )

# include(CMakePackageConfigHelpers)
# configure_package_config_file(
#   "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
#   INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
# )

# write_basic_package_version_file(
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
#   VERSION ${PROJECT_VERSION}
#   COMPATIBILITY SameMajorVersion
# )

# install(FILES
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
#   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
# )
