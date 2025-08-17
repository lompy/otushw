#pragma once
#include <ios>
#include <vector>
#include <istream>

template <class Stream>
Stream open_file(const char *path) {
    Stream s(path, std::ios::binary);

    if (!s.is_open())
        throw std::runtime_error("can't open file");

    return s;
}

template <class Stream>
Stream& rewind(Stream& s) {
    s.clear();
    if constexpr (std::is_base_of_v<std::istream, Stream>)
        s.seekg(0);
    if constexpr (std::is_base_of_v<std::ostream, Stream>)
        s.seekp(0);
    if (!s.good())
        throw std::runtime_error("can't rewind file");

    return s;
}
