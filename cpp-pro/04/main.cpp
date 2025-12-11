#include <cstdint>
#include <exception>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "lib.hpp"

int main() try {
    print_ip::print_ip(int8_t{-1});
    std::cout << "\n";
    print_ip::print_ip(int16_t{0});
    std::cout << "\n";
    print_ip::print_ip(int32_t{2130706433});
    std::cout << "\n";
    print_ip::print_ip(int64_t{8875824491850138409});
    std::cout << "\n";
    print_ip::print_ip("Hello, World!");
    std::cout << "\n";
    print_ip::print_ip(std::string{"Hello, World!"});
    std::cout << "\n";
    print_ip::print_ip(std::vector<int>{100, 200, 300, 400});
    std::cout << "\n";
    print_ip::print_ip(std::list<short>{400, 300, 200, 100});
    std::cout << "\n";
    print_ip::print_ip(std::make_tuple(123, 456, 789, 0));
    std::cout << "\n";

    return 0;
} catch (const std::exception& e) {
    std::cerr << e.what() << "\n";

    return 1;
}
