#define BOOST_TEST_MODULE test_version

#include "lib.hpp"

#include <boost/test/unit_test.hpp>
#include <tuple>
#include <vector>

BOOST_AUTO_TEST_SUITE(test_ip_filter)

BOOST_AUTO_TEST_CASE(test_ip_filter_parse) {
    BOOST_CHECK(ip_filter::ipv4::parse("1.2.3.4").first == ip_filter::ipv4({1, 2, 3, 4}));
    BOOST_CHECK(ip_filter::ipv4::parse("1.2.3.4").second == 7);

    BOOST_CHECK(ip_filter::ipv4::parse("1.0.30.43xxx").first == ip_filter::ipv4({1, 0, 30, 43}));
    BOOST_CHECK(ip_filter::ipv4::parse("1.0.30.43xxx").second == 9);

    BOOST_CHECK(ip_filter::ipv4::parse("255.0.21.40000").first ==
                ip_filter::ipv4({255, 0, 21, 40}));
    BOOST_CHECK(ip_filter::ipv4::parse("255.0.21.40000").second == 11);

    BOOST_CHECK(ip_filter::ipv4::parse("185.89.100.249	752").first ==
                ip_filter::ipv4({185, 89, 100, 249}));
    BOOST_CHECK(ip_filter::ipv4::parse("185.89.100.249	752").second == 14);

    // invalid
    BOOST_CHECK(ip_filter::ipv4::parse("00.0.21.40").second == 0);
    BOOST_CHECK(ip_filter::ipv4::parse("2550.0.21.40").second == 0);
    BOOST_CHECK(ip_filter::ipv4::parse("25.256.21.40").second == 0);
    BOOST_CHECK(ip_filter::ipv4::parse("255.0.21").second == 0);
    BOOST_CHECK(ip_filter::ipv4::parse("......10.").second == 0);
}

BOOST_AUTO_TEST_CASE(test_ip_filter_find) {
    ip_filter::sorted_ipv4_index ip_index{};
    ip_index.insert(ip_filter::ipv4({1, 2, 3, 4}));
    ip_index.insert(ip_filter::ipv4({1, 2, 3, 4}));
    ip_index.insert(ip_filter::ipv4({1, 2, 5, 4}));
    ip_index.insert(ip_filter::ipv4({2, 1, 5, 4}));
    ip_index.insert(ip_filter::ipv4({2, 4, 5, 4}));
    BOOST_CHECK(ip_index.size() == 5);

    std::vector<ip_filter::ipv4> result{};
    auto [it, end]{ip_index.find()};
    for (; it != end; it++)
        result.push_back(*it);
    BOOST_CHECK(result == (std::vector<ip_filter::ipv4>{
                              ip_filter::ipv4({2, 4, 5, 4}), ip_filter::ipv4({2, 1, 5, 4}),
                              ip_filter::ipv4({1, 2, 5, 4}), ip_filter::ipv4({1, 2, 3, 4}),
                              ip_filter::ipv4({1, 2, 3, 4})}));

    result = {};
    std::tie(it, end) = ip_index.find({ip_filter::sorted_ipv4_index::any_byte{4}});
    for (; it != end; it++)
        result.push_back(*it);
    BOOST_CHECK(result == (std::vector<ip_filter::ipv4>{
                              ip_filter::ipv4({2, 4, 5, 4}), ip_filter::ipv4({2, 1, 5, 4}),
                              ip_filter::ipv4({1, 2, 5, 4}), ip_filter::ipv4({1, 2, 3, 4}),
                              ip_filter::ipv4({1, 2, 3, 4})}));

    result = {};
    std::tie(it, end) = ip_index.find({ip_filter::sorted_ipv4_index::third_byte{5},
                                       ip_filter::sorted_ipv4_index::fourth_byte{4}});
    for (; it != end; it++) {
        result.push_back(*it);
    }
    BOOST_CHECK(result == (std::vector<ip_filter::ipv4>{ip_filter::ipv4({2, 4, 5, 4}),
                                                        ip_filter::ipv4({2, 1, 5, 4}),
                                                        ip_filter::ipv4({1, 2, 5, 4})}));

    result = {};
    std::tie(it, end) = ip_index.find(
        {ip_filter::sorted_ipv4_index::first_byte{1}, ip_filter::sorted_ipv4_index::third_byte{5}});
    for (; it != end; it++) {
        result.push_back(*it);
    }
    BOOST_CHECK(result == (std::vector<ip_filter::ipv4>{ip_filter::ipv4({1, 2, 5, 4})}));
}

BOOST_AUTO_TEST_SUITE_END()
