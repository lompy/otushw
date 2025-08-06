#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

#include "CRC32.hpp"
#include "IO.hpp"

void replaceLastFourBytes(std::vector<char> &data, uint32_t value) {
    std::copy_n(reinterpret_cast<const char *>(&value), 4, data.end() - 4);
}

std::vector<char> hack(const std::vector<char> &original,
                       const std::string &injection) {
    const uint32_t originalCrc32 = crc32(original.data(), original.size());

    std::vector<char> result(original.size() + injection.size() + 4);
    auto it = std::copy(original.begin(), original.end(), result.begin());
    std::copy(injection.begin(), injection.end(), it);

    const size_t maxVal = std::numeric_limits<uint32_t>::max();
    for (size_t i = 0; i < maxVal; ++i) {
        replaceLastFourBytes(result, uint32_t(i));
        auto currentCrc32 = crc32(result.data(), result.size());

        if (currentCrc32 == originalCrc32) {
            std::cout << "Success\n";
            return result;
        }
        if (i % 1000 == 0) {
            std::cout << "progress: "
                      << static_cast<double>(i) / static_cast<double>(maxVal)
                      << std::endl;
        }
    }
    throw std::logic_error("Can't hack");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Call with two args: " << argv[0]
                  << " <input file> <output file>\n";
        return 1;
    }

    try {
        const std::vector<char> data = readFromFile(argv[1]);
        const std::vector<char> badData = hack(data, "He-he-he");
        writeToFile(argv[2], badData);
    } catch (std::exception &ex) {
        std::cerr << ex.what() << '\n';
        return 2;
    }
    return 0;
}
