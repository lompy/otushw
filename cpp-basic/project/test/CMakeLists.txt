add_executable(squedl_test
  squedl_test.cpp
  test_bus_test.cpp
)

target_link_libraries(squedl_test
  PRIVATE
    squedl
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(squedl_test)
