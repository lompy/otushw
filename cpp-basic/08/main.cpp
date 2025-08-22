#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <string_view>
#include <numeric>
#include <random>
#include <functional>

#include "CRC32.hpp"
#include "IO.hpp"
#include "FirstValueWaiter.hpp"

constexpr size_t pad_size { sizeof(uint32_t) };
constexpr uint32_t pad_max_val { std::numeric_limits<uint32_t>::max() };
constexpr uint32_t pad_percent_range { (pad_max_val / 100) + 1 };
constexpr const char* default_injection { "He-he-he" };

std::optional<uint32_t> pad_crc32(uint32_t orig_crc32, const char* injection,
                                  uint32_t min = 0, uint32_t max = pad_max_val) {
    const auto inj_crc32 { crc32(injection, strlen(injection), ~orig_crc32) };

    for (auto i = min; i <= max; ++i)
        if (orig_crc32 == crc32(reinterpret_cast<const char *>(&i), pad_size, ~inj_crc32))
            return std::optional{i};

    return std::nullopt;
}

std::array<uint32_t, 50> random_percent_sequence() {
    std::array<uint32_t, 50> ranges;
    std::iota(ranges.begin(), ranges.end(), 0);
    std::transform(ranges.begin(), ranges.end(), ranges.begin(), [](uint32_t x) {
        return x * 2;
    });
    std::shuffle(ranges.begin(), ranges.end(), std::mt19937{std::random_device{}()});

    return ranges;
};

int main(int argc, char **argv) {
    if (argc != 3 && argc != 4) {
        std::cerr << "two or three args expected: " << argv[0]
                  << " <input file> <output file> [<injection>]" << std::endl;

        return 1;
    }

    try {
        const auto injection { argc == 4 ? argv[3] : default_injection };
        auto data { open_file<std::ifstream>(argv[1]) };
        const auto orig_crc32 { crc32(data) };

        FirstValueWaiter<uint32_t> waiter {};

        uint32_t min, max;
        for (auto percent: random_percent_sequence()) {
            min = pad_percent_range * percent;
            max = min + pad_percent_range > min ? min + pad_percent_range : pad_max_val;

            waiter.enqueue([orig_crc32, injection, min, max]() {
                return pad_crc32(orig_crc32, injection, min, max);
            });
        }

        const auto result { waiter.wait() };
        if (!result.has_value()) {
            std::cerr << "crc32 padding not found" << std::endl;

            return 2;
        }

        open_file<std::ofstream>(argv[2])
                << rewind(data).rdbuf()
                << injection
                << std::string_view {reinterpret_cast<const char *>(&result.value()), pad_size};
    } catch (std::exception &ex) {
        std::cerr << ex.what() << '\n';

        return 2;
    }

    return 0;
}
