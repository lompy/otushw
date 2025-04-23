#include <iostream>
#include <limits>
#include <ostream>

#include "arguments.hpp"
#include "randval.hpp"
#include "storage.hpp"

const char* max_option = "-max";
const char* level_option = "-level";
const char* table_option = "-table";

const char* file_option = "-file";
const char* range_option = "-range";
const char* help_option = "-help";

inline constexpr int default_level = 3;
inline constexpr int max_by_level[] = {0, 10, 50, 100};

const std::string default_file = "high_scores.txt";

const std::string usage = R"USAGE(
         -level X   Set maximum number according to level X, possible values: 1, 2, 3.
         -max N     Set maximum number to non negative N. Do not use with -level.
         -table     Show high scores table.
         -file name Use "name" as high scores table file.
         -range     Show possible range to player.
         -help      Show usage help.

)USAGE";

int print_usage_with_error_return()
{
    std::cout << "Unexpected option value." << usage;

    return 1;
}

int print_scores(storage::MinScoreFile &scores) {
    std::cout << "High scores table:" << std::endl << scores;

    return 0;
}

int main(int argc, char** argv)
{

    int help_idx = arguments::find_value_idx(argc, argv, help_option);
    if (help_idx != arguments::not_found) {
        std::cout << usage;

        return 0;
    }

    int max = max_by_level[default_level];

    int level_idx = arguments::find_value_idx(argc, argv, level_option);
    if (level_idx == arguments::found_no_value) {
        return print_usage_with_error_return();
    } else if (arguments::found_with_value(level_idx)) {
        int level = 0;
        try {
            level = std::stoi(argv[level_idx]);
        } catch (std::exception) {
            return print_usage_with_error_return();
        }

        if (level < 1 || level >= static_cast<int>(std::size(max_by_level)))
            return print_usage_with_error_return();

        max = max_by_level[level];
    }

    int max_idx = arguments::find_value_idx(argc, argv, max_option);
    if (max_idx == arguments::found_no_value ||
            (max_idx != arguments::not_found && level_idx != arguments::not_found)) {
        return print_usage_with_error_return();
    } else if (arguments::found_with_value(max_idx)) {
        try {
            max = std::stoi(argv[max_idx]);
        } catch (std::exception) {
            return print_usage_with_error_return();
        }

        if (max < 0)
            return print_usage_with_error_return();
    }

    std::string filename = default_file;
    int file_idx = arguments::find_value_idx(argc, argv, file_option);
    if (file_idx == arguments::found_no_value) {
        return print_usage_with_error_return();
    } else if (arguments::found_with_value(file_idx)) {
        filename = argv[file_idx];
    }
    storage::MinScoreFile scores;
    if (auto err = scores.open(filename); err) {
        std::cout << err->message << std::endl;

        return print_usage_with_error_return();
    }

    int table_idx = arguments::find_value_idx(argc, argv, table_option);
    if (table_idx == arguments::found_no_value) {
        return print_scores(scores);
    } else if (arguments::found_with_value(table_idx)) {
        return print_usage_with_error_return();
    }

    std::string enter_number = "Enter non negative number: ";
    int input_max = std::numeric_limits<int>::max();
    int range_idx = arguments::find_value_idx(argc, argv, range_option);
    if (range_idx == arguments::found_no_value) {
        enter_number = "Enter number from 0 to " + std::to_string(max) + ": ";
        input_max = max;
    } else if (arguments::found_with_value(range_idx)) {
        return print_usage_with_error_return();
    }

    const int target = randval::integer(max);
    int current_val = 0;
    int count = 0;

    std::string name;
    std::cout << "Enter your name: ";
    std::cin >> name;
    while (true) {
        count++;
        std::cout << enter_number;
        std::cin >> current_val;
        if (std::cin.fail() || current_val < 0 || current_val > input_max) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. ";

            continue;
        }

        if (current_val < target) {
            std::cout << current_val << " is less than the target." << std::endl;
        } else if (current_val > target) {
            std::cout << current_val << " is greater than the target." << std::endl;
        } else {
            std::cout << current_val << " is the target. You win!" << std::endl;

            break;
        }
    };

    scores.upsert(name, count);

    std::cout << "Your result is " << count << " guesses." << std::endl << std::endl;

    return print_scores(scores);
}
