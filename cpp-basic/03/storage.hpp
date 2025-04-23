#pragma once

#include <cstring>
#include <fstream>
#include <ostream>

namespace storage
{
struct Error {
    std::string message;
};

// MinScoreFile keeps minimum value for a key in a file.
// Make sure to call open before using.
class MinScoreFile
{
public:
    MinScoreFile() {}
    ~MinScoreFile()
    {
        if (file.is_open()) file.close();
    }
    MinScoreFile(const MinScoreFile&) = delete;
    MinScoreFile& operator=(const MinScoreFile&) = delete;

    // Opens file by given filename or creates one if not exists.
    // Returns Error if file is not opened.
    std::optional<Error> open(std::string_view filename);

    // Inserts or updates in place minimum value for a key.
    void upsert(std::string_view key, int value);

    friend std::ostream& operator<<(std::ostream& out, MinScoreFile& file);

private:
    static constexpr int int_len = std::numeric_limits<int>::digits10 + 2;
    static constexpr char separator = ' ';

    std::fstream file;
    void rewind()
    {
        file.clear();
        file.seekg(0);
    };
};
}
