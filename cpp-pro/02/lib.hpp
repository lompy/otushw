#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <set>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace ip_filter {

int version();

class ipv4 {
    std::array<unsigned char, 4> octets{};

public:
    ipv4() = default;
    explicit ipv4(std::array<unsigned char, 4> octets_v) : octets(octets_v) {};

    // returns ipv4 and num of chars occupied by it,
    // if num of chars == 0 then parse failed, ipv4 can be any value
    static std::pair<ipv4, size_t> parse(std::string_view str) {
        ipv4 result{};
        int candidate{};
        unsigned char num{};
        size_t i{};
        size_t octet{};
        bool is_zero{};
        for (i = 0; i < str.size(); ++i) {
            if (str[i] == '.') {
                octet++;
                is_zero = false;

                continue;
            }

            num = str[i] - '0';
            if (num > 9 || (num == 0 && is_zero && result.octets.at(octet) == 0))
                break;

            if (num == 0)
                is_zero = true;

            candidate = result.octets.at(octet) * 10 + num;
            if (candidate > 255)
                break;
            result.octets.at(octet) = candidate;
        }

        if (octet != 3)
            return {result, 0};

        return {result, i};
    };

    bool operator<(const ipv4& other) const { return octets < other.octets; };
    bool operator==(const ipv4& other) const { return octets == other.octets; };

    [[nodiscard]] unsigned char first() const { return octets[0]; };
    [[nodiscard]] unsigned char second() const { return octets[1]; };
    [[nodiscard]] unsigned char third() const { return octets[2]; };
    [[nodiscard]] unsigned char fourth() const { return octets[3]; };
};

inline std::ostream& operator<<(std::ostream& os, const ipv4& ip) {
    return os << static_cast<int>(ip.first()) << '.' << static_cast<int>(ip.second()) << '.'
              << static_cast<int>(ip.third()) << '.' << static_cast<int>(ip.fourth());
}

class sorted_ipv4_index {
    std::multiset<ipv4> all;

public:
    struct any {
        // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
        [[nodiscard]] bool match([[maybe_unused]] ipv4 ip) const { return true; };
    };
    struct first_byte {
        unsigned char val{};
        [[nodiscard]] bool match(ipv4 ip) const { return val == ip.first(); };
    };
    struct second_byte {
        unsigned char val{};
        [[nodiscard]] bool match(ipv4 ip) const { return val == ip.second(); };
    };
    struct third_byte {
        unsigned char val{};
        [[nodiscard]] bool match(ipv4 ip) const { return val == ip.third(); };
    };
    struct fourth_byte {
        unsigned char val{};
        [[nodiscard]] bool match(ipv4 ip) const { return val == ip.fourth(); };
    };
    struct any_byte {
        unsigned char val{};
        [[nodiscard]] bool match(ipv4 ip) const {
            return val == ip.first() || val == ip.second() || val == ip.third() ||
                   val == ip.fourth();
        };
    };

    using filter = std::variant<any, first_byte, second_byte, third_byte, fourth_byte, any_byte>;

    class iterator {
        std::multiset<ipv4>::const_reverse_iterator wrapped;
        std::multiset<ipv4>::const_reverse_iterator wrapped_end;
        std::vector<filter> included_in;

    public:
        iterator(std::multiset<ipv4>::const_reverse_iterator wrapped_v,
                 std::multiset<ipv4>::const_reverse_iterator wrapped_end_v,
                 std::vector<filter> included_in_v)
            : wrapped{wrapped_v}, wrapped_end{wrapped_end_v},
              included_in{std::move(included_in_v)} {
            while (!wrapped_is_included())
                ++wrapped;
        };

        auto operator*() const { return *wrapped; };
        auto operator->() { return wrapped.operator->(); };

        iterator& operator++() {
            if (wrapped == wrapped_end)
                return *this;

            ++wrapped;
            while (!wrapped_is_included())
                ++wrapped;

            return *this;
        };
        iterator operator++(int) {
            iterator tmp{*this};
            ++(*this);

            return tmp;
        };
        bool operator==(const iterator& other) const { return wrapped == other.wrapped; };
        bool operator!=(const iterator& other) const { return !(*this == other); }

    private:
        [[nodiscard]]
        bool wrapped_is_included() const {
            if (wrapped == wrapped_end || included_in.empty())
                return true;

            return std::all_of(included_in.cbegin(), included_in.cend(), [this](auto const& ftr) {
                return std::visit([this](auto&& ftr) { return ftr.match(*wrapped); }, ftr);
            });
        };
    };

    void insert(ipv4 val) { all.insert(val); };

    [[nodiscard]]
    std::pair<iterator, iterator> find(std::vector<filter> filters = {}) const {
        return {iterator{all.rbegin(), all.rend(), std::move(filters)},
                iterator{all.rend(), all.rend(), {}}};
    };

    size_t size() { return all.size(); };
};

}; // namespace ip_filter
