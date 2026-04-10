#include "frk/net/endian.hpp"

#include <stdint.h>

#include <bit>
#include <type_traits>

#include <doctest/doctest.h>

static_assert(std::endian::native == std::endian::little,
              "Fix the unit tests here if this is not LE machine");

////////////////////////////////////////////////////////////////////////////////

DOCTEST_TEST_SUITE_BEGIN("net/endian");

DOCTEST_TEST_CASE("net::to_le")
{
    // GIVEN
    constexpr uint8_t num8   = 0xaa;
    constexpr uint16_t num16 = 0xaabb;
    constexpr uint32_t num32 = 0xaabbccdd;
    constexpr uint64_t num64 = 0xaabbccddaabbccdd;

    // WHEN
    constexpr auto le_num8  = frk::net::to_le(num8);
    constexpr auto le_num16 = frk::net::to_le(num16);
    constexpr auto le_num32 = frk::net::to_le(num32);
    constexpr auto le_num64 = frk::net::to_le(num64);

    // THEN
    static_assert(std::is_same_v<decltype(le_num8), decltype(num8)>);
    static_assert(std::is_same_v<decltype(le_num16), decltype(num16)>);
    static_assert(std::is_same_v<decltype(le_num32), decltype(num32)>);
    static_assert(std::is_same_v<decltype(le_num64), decltype(num64)>);

    static_assert(num8 == le_num8);
    static_assert(num16 == le_num16);
    static_assert(num32 == le_num32);
    static_assert(num64 == le_num64);

    DOCTEST_CHECK_EQ(num8, le_num8);
    DOCTEST_CHECK_EQ(num16, le_num16);
    DOCTEST_CHECK_EQ(num32, le_num32);
    DOCTEST_CHECK_EQ(num64, le_num64);
}

DOCTEST_TEST_CASE("net::to_be")
{
    // GIVEN
    constexpr uint8_t num8   = 0xaa;
    constexpr uint16_t num16 = 0xaabb;
    constexpr uint32_t num32 = 0xaabbccdd;
    constexpr uint64_t num64 = 0xaabbccddaabbccdd;

    // WHEN
    constexpr auto be_num8  = frk::net::to_be(num8);
    constexpr auto be_num16 = frk::net::to_be(num16);
    constexpr auto be_num32 = frk::net::to_be(num32);
    constexpr auto be_num64 = frk::net::to_be(num64);

    // THEN
    static_assert(std::is_same_v<decltype(be_num8), decltype(num8)>);
    static_assert(std::is_same_v<decltype(be_num16), decltype(num16)>);
    static_assert(std::is_same_v<decltype(be_num32), decltype(num32)>);
    static_assert(std::is_same_v<decltype(be_num64), decltype(num64)>);

    static_assert(num8 == be_num8);
    static_assert(std::byteswap(num16) == be_num16);
    static_assert(std::byteswap(num32) == be_num32);
    static_assert(std::byteswap(num64) == be_num64);

    DOCTEST_CHECK_EQ(num8, be_num8);
    DOCTEST_CHECK_EQ(std::byteswap(num16), be_num16);
    DOCTEST_CHECK_EQ(std::byteswap(num32), be_num32);
    DOCTEST_CHECK_EQ(std::byteswap(num64), be_num64);
}

DOCTEST_TEST_CASE("net::from_le")
{
    // GIVEN
    constexpr uint8_t num8   = 0xaa;
    constexpr uint16_t num16 = 0xaabb;
    constexpr uint32_t num32 = 0xaabbccdd;
    constexpr uint64_t num64 = 0xaabbccddaabbccdd;

    // WHEN
    constexpr auto host_num8  = frk::net::from_le(num8);
    constexpr auto host_num16 = frk::net::from_le(num16);
    constexpr auto host_num32 = frk::net::from_le(num32);
    constexpr auto host_num64 = frk::net::from_le(num64);

    // THEN
    static_assert(std::is_same_v<decltype(host_num8), decltype(num8)>);
    static_assert(std::is_same_v<decltype(host_num16), decltype(num16)>);
    static_assert(std::is_same_v<decltype(host_num32), decltype(num32)>);
    static_assert(std::is_same_v<decltype(host_num64), decltype(num64)>);

    static_assert(num8 == host_num8);
    static_assert(num16 == host_num16);
    static_assert(num32 == host_num32);
    static_assert(num64 == host_num64);

    DOCTEST_CHECK_EQ(num8, host_num8);
    DOCTEST_CHECK_EQ(num16, host_num16);
    DOCTEST_CHECK_EQ(num32, host_num32);
    DOCTEST_CHECK_EQ(num64, host_num64);
}

DOCTEST_TEST_CASE("net::from_be")
{
    // GIVEN
    constexpr uint8_t num8   = 0xaa;
    constexpr uint16_t num16 = 0xaabb;
    constexpr uint32_t num32 = 0xaabbccdd;
    constexpr uint64_t num64 = 0xaabbccddaabbccdd;

    // WHEN
    constexpr auto host_num8  = frk::net::from_be(num8);
    constexpr auto host_num16 = frk::net::from_be(num16);
    constexpr auto host_num32 = frk::net::from_be(num32);
    constexpr auto host_num64 = frk::net::from_be(num64);

    // THEN
    static_assert(std::is_same_v<decltype(host_num8), decltype(num8)>);
    static_assert(std::is_same_v<decltype(host_num16), decltype(num16)>);
    static_assert(std::is_same_v<decltype(host_num32), decltype(num32)>);
    static_assert(std::is_same_v<decltype(host_num64), decltype(num64)>);

    static_assert(num8 == host_num8);
    static_assert(std::byteswap(num16) == host_num16);
    static_assert(std::byteswap(num32) == host_num32);
    static_assert(std::byteswap(num64) == host_num64);

    DOCTEST_CHECK_EQ(num8, host_num8);
    DOCTEST_CHECK_EQ(std::byteswap(num16), host_num16);
    DOCTEST_CHECK_EQ(std::byteswap(num32), host_num32);
    DOCTEST_CHECK_EQ(std::byteswap(num64), host_num64);
}

DOCTEST_TEST_SUITE_END();
