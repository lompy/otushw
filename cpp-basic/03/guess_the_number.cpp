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

int print_usage_with_result(int result)
{
    std::cout << "Unexpected option value." << usage;

    return result;
}

int print_scores_table(storage::MinScoreFile &scores)
{
    std::cout << "High scores table:" << std::endl << scores;

    return 0;
}

// { max, ok }
std::pair<int, bool> max_from_args(int argc, char** argv)
{
    int level = default_level;

    int level_idx = arguments::find_value_idx(argc, argv, level_option);
    if (level_idx == arguments::found_no_value)
        return {0, false};
    if (arguments::found_with_value(level_idx))
        try {
            level = std::stoi(argv[level_idx]);
        } catch (std::exception) {
            return {0, false};
        }

    if (level < 1 || level >= static_cast<int>(std::size(max_by_level)))
        return {0, false};

    int max = max_by_level[level];
    int max_idx = arguments::find_value_idx(argc, argv, max_option);
    if (max_idx == arguments::found_no_value ||
            (max_idx != arguments::not_found && level_idx != arguments::not_found))
        return {0, false};

    if (arguments::found_with_value(max_idx))
        try {
            max = std::stoi(argv[max_idx]);
        } catch (std::exception) {
            return {0, false};
        }
    if (max < 0)
        return {0, false};

    return {max, true};
}

// { filename, ok }
std::pair<std::string, bool> filename_from_args(int argc, char** argv)
{
    std::string filename = default_file;
    int file_idx = arguments::find_value_idx(argc, argv, file_option);
    if (file_idx == arguments::found_no_value)
        return {"", false};
    if (arguments::found_with_value(file_idx))
        filename = argv[file_idx];

    return {filename, true};
}

int play(int target, int input_max, std::string_view enter_number)
{
    int current_val = 0;
    int count = 0;

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

    return count;
}

int main(int argc, char** argv)
{
    if (arguments::has_option(argc, argv, help_option))
        return print_usage_with_result(0);

    auto [max, max_ok] = max_from_args(argc, argv);
    if (!max_ok)
        return print_usage_with_result(1);

    auto [filename, filename_ok] = filename_from_args(argc, argv);
    if (!filename_ok)
        return print_usage_with_result(1);

    storage::MinScoreFile scores;
    if (auto err = scores.open(filename); err) {
        std::cout << err->message << std::endl;

        return print_usage_with_result(1);
    }

    if (arguments::has_option(argc, argv, table_option))
        return print_scores_table(scores);

    std::string enter_number = "Enter non negative number: ";
    int input_max = std::numeric_limits<int>::max();
    if (arguments::has_option(argc, argv, range_option)) {
        enter_number = "Enter number from 0 to " + std::to_string(max) + ": ";
        input_max = max;
    }

    std::string name;
    std::cout << "Enter your name: ";
    std::cin >> name;

    const int target = randval::integer(max);
    int count = play(target, input_max, enter_number);
    scores.upsert(name, count);
    std::cout << "Your result is " << count << " guesses." << std::endl << std::endl;

    return print_scores_table(scores);
}
