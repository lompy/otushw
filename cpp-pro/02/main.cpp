#include <cstddef>
#include <exception>
#include <iostream>
#include <string>
#include <tuple>

#include "lib.hpp"

int main() try {
    ip_filter::sorted_ipv4_index ip_index{};
    size_t i{};
    for (std::string line; std::getline(std::cin, line);) {
        i++;
        auto [ip, next] = ip_filter::ipv4::parse(line);
        if (next == 0 || next >= line.size() || line[next] != '\t') {
            std::cerr << "parse line " << i << ": " << line << "\n";
            continue;
        }

        ip_index.insert(ip);
    }

    auto [it, end]{ip_index.find()};
    for (; it != end; it++)
        std::cout << *it << "\n";

    std::tie(it, end) = ip_index.find({ip_filter::sorted_ipv4_index::first_byte{1}});
    for (; it != end; it++)
        std::cout << *it << "\n";

    std::tie(it, end) = ip_index.find({ip_filter::sorted_ipv4_index::first_byte{46},
                                       ip_filter::sorted_ipv4_index::second_byte{70}});
    for (; it != end; it++)
        std::cout << *it << "\n";

    std::tie(it, end) = ip_index.find({ip_filter::sorted_ipv4_index::any_byte{46}});
    for (; it != end; it++)
        std::cout << *it << "\n";

    return 0;
} catch (const std::exception& e) {
    std::cerr << e.what() << "\n";

    return 1;
}
