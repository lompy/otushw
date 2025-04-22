#include <cstdlib>
#include <ctime>

#include "randval.hpp"

int randval::integer(int max)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    return std::rand() % max;
}
