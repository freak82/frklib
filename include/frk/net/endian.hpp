#pragma once

#include <bit>
#include <concepts>

namespace frk::net
{

template <std::unsigned_integral T>
[[nodiscard]] constexpr auto to_le(T v) noexcept
{
    if constexpr (std::endian::native == std::endian::big)
        return std::byteswap(v);
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

template <std::unsigned_integral T>
[[nodiscard]] constexpr auto from_le(T v) noexcept
{
    if constexpr (std::endian::native == std::endian::big)
        return std::byteswap(v);
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

} // namespace frk::net
