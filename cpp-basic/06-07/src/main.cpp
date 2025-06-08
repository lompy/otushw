#include <cstddef>
#include <iostream>
#include "sutovec.hpp"
#include "sutolist.hpp"

int main() {
    Sutovec<int> sutovec;

    sutovec.push_back(10);
    sutovec.push_back(20);
    sutovec.push_back(30);
    sutovec.push_back(40);
    sutovec.push_back(50);
    sutovec.erase(2);

    for (auto it : sutovec) {
        std::cout << it << std::endl;
    }

    std::cout << sutovec << std::endl;

    Sutovec<int> sutovec2 { sutovec };
    std::cout << sutovec2 << std::endl;
    sutovec2.push_back(1000);
    sutovec2[2] = 2000;
    std::cout << sutovec << std::endl;
    std::cout << sutovec2 << std::endl;

    std::cout << Sutovec<int>(10, 20) << std::endl;

    std::cout << "=====================" << std::endl;

    Sutolist<int> sutolist;

    sutolist.push_back(10);
    sutolist.push_back(20);
    sutolist.push_back(30);
    sutolist.push_back(40);
    sutolist.push_back(50);
    sutolist.erase(2);

    for (auto it : sutolist) {
        std::cout << it << std::endl;
    }

    std::cout << sutovec << std::endl;

    Sutolist<int> sutolist2 { sutolist };
    std::cout << sutolist2 << std::endl;
    sutolist2.push_back(1000);
    sutolist2[2] = 2000;
    std::cout << sutolist << std::endl;
    std::cout << sutolist2 << std::endl;

    std::cout << Sutolist{10, 20} << std::endl;

    for (auto it = sutolist.begin(); it != sutolist.end(); ++it)
        std::cout << *it << " ";

    return 0;
}
