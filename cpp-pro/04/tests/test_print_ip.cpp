#include <cstdint>
#include <list>
#include <string>
#include <vector>

#define BOOST_TEST_MODULE test_version

#include "lib.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_print_ip)

BOOST_AUTO_TEST_CASE(test_print_ip_to_string) {
    BOOST_CHECK(print_ip::to_string(int8_t{-1}) == "255");
    BOOST_CHECK(print_ip::to_string(int16_t{0}) == "0.0");
    BOOST_CHECK(print_ip::to_string(int32_t{2130706433}) == "127.0.0.1");
    BOOST_CHECK(print_ip::to_string(int64_t{8875824491850138409}) == "123.45.67.89.101.112.131.41");
    BOOST_CHECK(print_ip::to_string("Hello, World!") == "Hello, World!");
    BOOST_CHECK(print_ip::to_string(std::string{"Hello, World!"}) == "Hello, World!");
    BOOST_CHECK(print_ip::to_string(std::vector<int>{100, 200, 300, 400}) == "100.200.300.400");
    BOOST_CHECK(print_ip::to_string(std::list<short>{400, 300, 200, 100}) == "400.300.200.100");
    BOOST_CHECK(print_ip::to_string(std::make_tuple(123, 456, 789, 0)) == "123.456.789.0");
}

BOOST_AUTO_TEST_SUITE_END()
