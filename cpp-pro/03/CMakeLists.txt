cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(sutoloc_lib VERSION ${PROJECT_VESRION})

option(WITH_BOOST_TEST "Whether to build Boost test" ON)

configure_file(version.hpp.in version.hpp)

add_executable(sutoloc main.cpp)
add_library(sutoloc_lib lib.cpp)

set_target_properties(sutoloc sutoloc_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(sutoloc_lib
    PRIVATE "${CMAKE_BINARY_DIR}"
)

target_link_libraries(sutoloc PRIVATE
    sutoloc_lib
)

if(WITH_BOOST_TEST)
    find_package(Boost COMPONENTS unit_test_framework REQUIRED)

    add_executable(test_version test_version.cpp)
    set_target_properties(test_version PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    set_target_properties(test_version PROPERTIES
        COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
    )
    target_link_libraries(test_version
        ${Boost_LIBRARIES}
        sutoloc_lib
    )

    add_executable(test_sutoloc test_sutoloc.cpp)
    set_target_properties(test_sutoloc PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    set_target_properties(test_sutoloc PROPERTIES
        COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
    )
    target_link_libraries(test_sutoloc
        ${Boost_LIBRARIES}
        sutoloc_lib
    )
endif()

if (MSVC)
    target_compile_options(sutoloc PRIVATE
        /W4
    )
    target_compile_options(sutoloc_lib PRIVATE
        /W4
    )
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE
            /W4
        )
        target_compile_options(test_sutoloc PRIVATE
            /W4
        )
    endif()
else ()
    target_compile_options(sutoloc PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    target_compile_options(sutoloc_lib PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE
            -Wall -Wextra -pedantic -Werror
        )
    endif()
endif()

install(TARGETS sutoloc RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)

if(WITH_BOOST_TEST)
    enable_testing()
    add_test(test_version test_version)
    add_test(test_sutoloc test_sutoloc)
endif()
