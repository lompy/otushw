#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <utility>

#include "lib.hpp"

namespace {
template <typename T>
void print_map(const T& container) {
    if (container.empty()) {
        std::cout << "empty";
        return;
    }

    std::cout << container.at(0);
    for (size_t i{1}; i < container.size(); ++i)
        std::cout << " " << container.at(i);

    std::cout << "\n";
}

template <typename T>
void print_list(const T& container) {
    if (container.empty()) {
        std::cout << "empty";
        return;
    }

    std::cout << *container.begin();
    for (auto it{container.begin() + 1}; it != container.end(); ++it)
        std::cout << " " << *it;

    std::cout << "\n";
}

template <typename T>
void fill(T& container) {
    container[0] = 1;
    for (int i{1}; i < 10; ++i)
        container[i] = i * container[i - 1];
}
} // namespace

int main() try {
    std::cout << "std map" << "\n";
    std::map<int, int> std_map{};
    fill(std_map);
    print_map(std_map);

    std::cout << "std map with sutoloc::allocator" << "\n";
    std::map<int, int, std::less<>, sutoloc::allocator<std::pair<const int, int>, 10>>
        sutoloc_std_map{};
    fill(sutoloc_std_map);
    print_map(sutoloc_std_map);

    std::cout << "sutoloc::list" << "\n";
    sutoloc::list<int> sutoloc_list{};
    for (int i{0}; i < 10; ++i)
        sutoloc_list.push_back(i);
    print_list(sutoloc_list);

    std::cout << "sutoloc::list with sutoloc::allocator" << "\n";
    sutoloc::list<int, sutoloc::allocator<int, 10>> sutoloc_allocator_list{};
    for (int i{0}; i < 10; ++i)
        sutoloc_allocator_list.push_back(i);
    print_list(sutoloc_allocator_list);

    return 0;
} catch (const std::exception& e) {
    std::cerr << e.what() << "\n";

    return 1;
}
