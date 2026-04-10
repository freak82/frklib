#pragma once

#include <cstring>
#include <span>
#include <type_traits>

namespace frk::algo
{

template <typename T>
    requires(std::is_trivially_copyable_v<T> &&
             std::has_unique_object_representations_v<T>)
int mem_compare(const T& lhs, const T& rhs) noexcept
{
    return std::memcmp(&lhs, &rhs, sizeof(T));
}

template <typename T>
    requires(std::is_trivially_copyable_v<T> &&
             std::has_unique_object_representations_v<T>)
int mem_compare(const T& lhs,
                std::span<const unsigned char, sizeof(T)> rhs) noexcept
{
    return std::memcmp(&lhs, rhs.data(), sizeof(T));
}

template <typename T>
    requires(std::is_trivially_copyable_v<T> &&
             std::has_unique_object_representations_v<T>)
int mem_compare(std::span<const unsigned char, sizeof(T)> lhs,
                const T& rhs) noexcept
{
    return std::memcmp(lhs.data(), &rhs, sizeof(T));
}

} // namespace frk::algo
