#pragma once

#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#if (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || defined(_WIN32)
#else
#error "only little endian is supported!"
#endif

namespace print_ip {

int version();

namespace detail {

using byte = unsigned char;

struct little_endian_byte_span {
    using const_iterator = std::reverse_iterator<const byte*>;

    template <class T>
    explicit little_endian_byte_span(const T& value) noexcept
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
        : first{reinterpret_cast<const byte*>(std::addressof(value))}, last{first + sizeof(T)} {}

    [[nodiscard]] const_iterator cbegin() const noexcept { return const_iterator{last}; };

    [[nodiscard]] const_iterator cend() const noexcept { return const_iterator{first}; };

private:
    const byte* first;
    const byte* last;
};

template <typename T>
struct is_str : std::false_type {};

template <typename Char, typename Traits, typename Alloc>
struct is_str<std::basic_string<Char, Traits, Alloc>> : std::true_type {};

template <typename Char, typename Traits>
struct is_str<std::basic_string_view<Char, Traits>> : std::true_type {};

template <typename Char>
struct is_str<Char*> : std::bool_constant<std::is_same_v<std::remove_cv_t<Char>, char>> {};

template <typename Char, size_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
struct is_str<Char[N]> : is_str<Char*> {};

template <typename T>
constexpr bool is_str_v = is_str<T>::value;

template <typename T>
struct is_homo_tuple : std::false_type {};

template <typename Head, typename... Tail>
struct is_homo_tuple<std::tuple<Head, Tail...>> : std::conjunction<std::is_same<Head, Tail>...> {};

template <typename... Ts>
constexpr bool is_homo_tuple_v = is_homo_tuple<Ts...>::value;

}; // namespace detail

template <typename T>
std::enable_if_t<!detail::is_str_v<T>, std::void_t<typename T::const_iterator>>
to_stream(std::ostream& out, const T& vals) {
    auto first{true};
    for (auto it{vals.cbegin()}; it != vals.cend(); ++it)
        out << (std::exchange(first, false) ? "" : ".") << static_cast<long long>(*it);
};

template <typename T>
std::enable_if_t<std::is_scalar_v<T>> to_stream(std::ostream& out, const T& val) {
    to_stream(out, detail::little_endian_byte_span{val});
};

template <typename T>
std::enable_if_t<detail::is_str_v<T>> to_stream(std::ostream& out, const T& str) {
    out << str;
};

template <typename... Ts>
std::enable_if_t<detail::is_homo_tuple_v<std::tuple<Ts...>> && (sizeof...(Ts) > 0)>
to_stream(std::ostream& out, const std::tuple<Ts...>& tup) {
    auto first{true};
    std::apply(
        [&](const auto&... vals) {
            ((out << (std::exchange(first, false) ? "" : ".") << static_cast<long long>(vals)),
             ...);
        },
        tup);

    // If code reuse needed it is possible but quite hard to implement const_iterator over tuple.
    //
    // If we can afford allocation, then we can convert to vector and again reuse iterator.
    //
    // std::vector<std::tuple_element_t<0, std::tuple<Ts...>>> vec;
    // vec.reserve(sizeof...(Ts));
    // std::apply([&](const auto&... vals) { (vec.push_back(vals), ...); }, tup);
    // to_stream(out, vec);
};

template <typename T>
std::string to_string(const T& val) {
    std::ostringstream out{};
    to_stream(out, val);

    return out.str();
};

template <typename T>
void print_ip(const T& val) {
    to_stream(std::cout, val);
};

}; // namespace print_ip
