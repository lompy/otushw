#pragma once

namespace arguments
{
inline constexpr char prefix = '-';
inline constexpr int not_found = -1;
inline constexpr int found_no_value = 0;

/**
 * Returns index of option value in argv array,
 * 0 if option is found but no value provided,
 * -1 if option is not found,
 * -1 if option doesn't have '-' prefix or empty.
 *
 * Option is not expected to be found in 0 position of argv.
 */
int find_value_idx(int argc, char **argv, const char* option);

bool has_option(int argc, char** argv, const char* option);
bool found_with_value(int value_idx);
}
