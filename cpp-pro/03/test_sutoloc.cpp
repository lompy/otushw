#include <list>
#include <vector>

#define BOOST_TEST_MODULE test_version

#include "lib.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_sutoloc)

BOOST_AUTO_TEST_CASE(test_sutoloc_with_std) {
    std::vector<int, sutoloc::allocator<int, 8>> vec{};

    vec.push_back(0);
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);
    vec.push_back(6);
    vec.push_back(7);
    vec.push_back(8);
    vec.push_back(9);

    BOOST_CHECK(vec ==
                (std::vector<int, sutoloc::allocator<int, 8>>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));

    vec.clear();

    vec.push_back(0);
    vec.push_back(1);
    vec.push_back(2);

    BOOST_CHECK(vec == (std::vector<int, sutoloc::allocator<int, 8>>{0, 1, 2}));
}

BOOST_AUTO_TEST_SUITE_END()
