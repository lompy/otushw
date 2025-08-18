// Read files and prints top k word by frequency

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <thread>
#include <vector>
#include <chrono>

using Counter = std::map<std::string, std::size_t>;

const long TOPK { 10 };

std::string tolower(const std::string& str);

Counter count_words(std::istream& stream);

void print_topk(std::ostream& out, const Counter& c, const long k);

std::ifstream open(const char *path);

int main(int argc, char** argv) {
    std::size_t num_files { static_cast<std::size_t>(argc) - 1 };
    if (num_files <= 0) {
        std::cerr << "usage: " << argv[0] << " [FILES...]" << std::endl;

        return EXIT_FAILURE;
    }

    auto start { std::chrono::high_resolution_clock::now() };

    std::vector<Counter> res { num_files };
    std::vector<std::thread> threads {};
    threads.reserve(num_files);

    for (size_t i {}; i < num_files; ++i)
        threads.emplace_back([i, in = open(argv[i+1]), &res]() mutable { res[i] = count_words(in); });

    for (auto &t : threads)
        t.join();

    Counter total {};
    for (const auto& per_file : res)
        for (const auto& pair: per_file)
            total[pair.first] += pair.second;

    print_topk(std::cout, total, TOPK);
    auto end { std::chrono::high_resolution_clock::now() };
    auto elapsed_ms { std::chrono::duration_cast<std::chrono::microseconds>(end - start) };
    std::cout << "elapsed time is " << elapsed_ms.count() << " us" << std::endl;
};

std::ifstream open(const char *path) {
    std::ifstream s { path };

    if (!s.is_open())
        throw std::runtime_error("can't open file");

    return s;
};

std::string tolower(const std::string &str) {
    std::string lower_str;
    std::transform(std::cbegin(str), std::cend(str),
                   std::back_inserter(lower_str),
    [](unsigned char ch) {
        return std::tolower(ch);
    });

    return lower_str;
};

Counter count_words(std::istream& stream) {
    Counter counter {};
    std::string word {};
    while (stream >> word)
        ++counter[tolower(word)];

    return counter;
};

void print_topk(std::ostream& stream, const Counter& counter, const long k) {
    std::vector<Counter::const_iterator> words;
    words.reserve(counter.size());
    for (auto it { std::cbegin(counter) }; it != std::cend(counter); ++it)
        words.push_back(it);

    std::partial_sort(std::begin(words),
                      std::begin(words) + k,
                      std::end(words),
    [](auto lhs, auto &rhs) {

        return lhs->second > rhs->second;
    });

    for (std::size_t i = 0; i < static_cast<std::size_t>(k); ++i)
        stream << std::setw(4) << words[i]->second << " " << words[i]->first << std::endl;
};
