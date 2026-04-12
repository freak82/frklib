#pragma once

#include <bit>
#include <concepts>

namespace frk::net
{
// Silence pedantic warning for ISO C++ not supporting __int128
__extension__ using uint128_t = unsigned __int128;

// The libstdc++ implementation of std::byteswap has support for 128 bit
// integers but it's constrained to work with integral types and these don't
// include 128 bit integers and thus the function can't be called.
[[nodiscard]] constexpr uint128_t byteswap_u128(uint128_t v) noexcept
{
#if __has_builtin(__builtin_bswap128)
    return __builtin_bswap128(v);
#else
    return (__builtin_bswap64(v >> 64) |
            (static_cast<uint128_t>(__builtin_bswap64(v)) << 64));
#endif
}

template <std::unsigned_integral T>
[[nodiscard]] constexpr auto to_le(T v) noexcept
{
    if constexpr (std::endian::native == std::endian::big)
        return std::byteswap(v);
    else
        return v;
}

[[nodiscard]] constexpr uint128_t to_le(uint128_t v) noexcept
{
    if constexpr (std::endian::native == std::endian::big)
        return frk::net::byteswap_u128(v);
    else
        return v;
}

template <std::unsigned_integral T>
[[nodiscard]] constexpr auto to_be(T v) noexcept
{
    if constexpr (std::endian::native == std::endian::little)
        return std::byteswap(v);
    else
        return v;
}

[[nodiscard]] constexpr uint128_t to_be(uint128_t v) noexcept
{
    if constexpr (std::endian::native == std::endian::little)
        return frk::net::byteswap_u128(v);
    else
        return v;
}

template <std::unsigned_integral T>
[[nodiscard]] constexpr auto from_le(T v) noexcept
{
    if constexpr (std::endian::native == std::endian::big)
        return std::byteswap(v);
    else
        return v;
}

[[nodiscard]] constexpr uint128_t from_le(uint128_t v) noexcept
{
    if constexpr (std::endian::native == std::endian::big)
        return frk::net::byteswap_u128(v);
    else
        return v;
}

template <std::unsigned_integral T>
[[nodiscard]] constexpr auto from_be(T v) noexcept
{
    if constexpr (std::endian::native == std::endian::little)
        return std::byteswap(v);
    else
        return v;
}

[[nodiscard]] constexpr uint128_t from_be(uint128_t v) noexcept
{
    if constexpr (std::endian::native == std::endian::little)
        return frk::net::byteswap_u128(v);
    else
        return v;
}

} // namespace frk::net
