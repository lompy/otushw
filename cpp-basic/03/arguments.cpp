#include <cstring>

#include "arguments.hpp"

int arguments::find_value_idx(int argc, char** argv, const char* option)
{
    if (argc <= 1 || std::strlen(option) < 2 || option[0] != arguments::prefix)
        return arguments::not_found;

    int opt_idx = 0;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(option, argv[i]) == 0) {
            opt_idx = i;
            break;
        }
    }

    if (opt_idx == 0) {
        return arguments::not_found;
    } else if (opt_idx == argc-1 || argv[opt_idx+1][0] == arguments::prefix) {
        return arguments::found_no_value;
    } else {
        return opt_idx + 1;
    }
}

bool arguments::has_option(int argc, char** argv, const char* option)
{
    return arguments::find_value_idx(argc, argv, option) != arguments::not_found;
}

bool arguments::found_with_value(int value_idx)
{
    return value_idx > 0;
}
