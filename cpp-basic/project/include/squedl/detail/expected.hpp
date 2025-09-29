#ifndef SQUEDL_DETAIL_EXPECTED_HPP
#define SQUEDL_DETAIL_EXPECTED_HPP

#if __cplusplus >= 202302L
    #include <expected>
    namespace squedl::detail {
        template<typename T, typename E>
        using expected = std::expected<T, E>;
        
        template<typename E>
        using unexpected = std::unexpected<E>;
        
        template<typename E>
        auto make_unexpected(E&& error_value) {
            return std::make_unexpected(std::forward<E>(error_value));
        }
    } // namespace squedl::detail
#else
    #include <nonstd/expected.hpp>
    namespace squedl::detail {
        template<typename T, typename E>
        using expected = nonstd::expected<T, E>;
        
        template<typename E>
        using unexpected = nonstd::unexpected_type<E>;
        
        template<typename E>
        auto make_unexpected(E&& error_value) {
            return nonstd::make_unexpected(std::forward<E>(error_value));
        }
    } // namespace squedl::detail
#endif

#endif // SQUEDL_DETAIL_EXPECTED_HPP
