#include <cstring>
#include <fstream>
#include <iosfwd>
#include <optional>
#include <ostream>
#include <string>
#include <iostream>

#include "storage.hpp"

std::optional<storage::Error> storage::MinScoreFile::open(std::string_view filename)
{
    // can't use std::ios::app here as I need to write in place, not always append
    file.open(filename, std::ios::in | std::ios::out);

    if (file.is_open())
        return std::nullopt;

    file.clear();
    file.open(filename, std::ios::in | std::ios::out | std::ios::trunc);

    if (file.is_open())
        return std::nullopt;

    if (file.fail())
        return Error{"file not opened: " + std::string(strerror(errno))};

    return Error{"file not opened"};
}

std::ostream& storage::operator<<(std::ostream& out, storage::MinScoreFile& file)
{
    file.rewind();

    std::string line;
    while (std::getline(file.file, line)) out << line << std::endl;

    file.rewind();

    return out;
}

void storage::MinScoreFile::upsert(std::string_view key, int value)
{
    rewind();

    std::string line;
    bool found = false;

    while (std::getline(file, line)) {
        if (line.length() != key.length() + 1 + int_len || line.rfind(key, 0) != 0)
            continue;

        found = true;

        int current_value = std::numeric_limits<int>::max();
        try {
            current_value = std::stoi(line.substr(key.length()));
        } catch (std::exception) {}

        if (value < current_value) {
            file.seekp(-(int_len + 1), std::ios::cur);
            file << std::setw(int_len) << value;
        }

        break;
    }

    if (!found) {
        file.clear();
        file << key << separator << std::setw(int_len) << value << std::endl;
    }

    rewind();
}
