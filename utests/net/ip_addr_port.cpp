#include "frk/algo/algorithm.hpp"
#include "frk/algo/compare.hpp"
#include "frk/net/ip_addr_port.hpp"

#include <doctest/doctest.h>

#include <bit>
#include <format>
#include <span>
#include <string_view>

DOCTEST_TEST_SUITE_BEGIN("net/ip_addr_port");

DOCTEST_TEST_CASE("net::ip4_addr_port::from_be(integer)")
{
    // GIVEN
    constexpr frk::net::ip4_addr::integer_type addr = 0x0A000001u;
    constexpr auto port = frk::net::to_be(uint16_t(8080));

    // WHEN
    const auto addr_port = frk::net::ip4_addr_port::from_be(addr, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.to_be_integer(), addr);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("net::ip4_addr_port::from_be(span)")
{
    // GIVEN
    constexpr unsigned char addr[] = {192, 168, 10, 42};
    constexpr auto port            = frk::net::to_be(uint16_t(8080));

    // WHEN
    const auto addr_port =
        frk::net::ip4_addr_port::from_be(std::span{addr}, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[0], 192);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[1], 168);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[2], 10);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[3], 42);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("net::ip4_addr_port::from_be(in_addr)")
{
    // GIVEN
    constexpr unsigned char repr[] = {192, 168, 10, 42};
    constexpr auto addr            = std::bit_cast<in_addr>(repr);
    constexpr auto port            = frk::net::to_be(uint16_t(8080));

    // WHEN
    const auto addr_port = frk::net::ip4_addr_port::from_be(addr, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[0], 192);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[1], 168);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[2], 10);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[3], 42);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("net::ip4_addr_port::from_native")
{
    // GIVEN
    constexpr frk::net::ip4_addr::integer_type addr =
        frk::net::from_be(0x0A000001u);
    constexpr uint16_t port = 8080;

    // WHEN
    const auto addr_port = frk::net::ip4_addr_port::from_native(addr, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.to_native_integer(), addr);
    DOCTEST_CHECK_EQ(frk::net::from_be(addr_port.port_), port);
}

DOCTEST_TEST_CASE("net::ip4_addr_port::to_sockaddr_in")
{
    // GIVEN
    constexpr frk::net::ip4_addr::integer_type addr =
        frk::net::from_be(0xC0A80A2Au);
    constexpr uint16_t port = 8080;
    const auto addr_port    = frk::net::ip4_addr_port::from_native(addr, port);

    // WHEN
    const auto saddr = addr_port.to_sockaddr_in();

    // THEN
    DOCTEST_CHECK_EQ(saddr.sin_family, AF_INET);
    DOCTEST_CHECK_EQ(frk::net::from_be(saddr.sin_port), port);
    DOCTEST_CHECK_EQ(saddr.sin_addr.s_addr,
                     addr_port.addr_.to_in_addr().s_addr);
    DOCTEST_CHECK(frk::algo::all_of(saddr.sin_zero, '\0'));
}

DOCTEST_TEST_CASE("net::ip4_addr_port::operator==")
{
    // GIVEN
    constexpr frk::net::ip4_addr::integer_type addr =
        frk::net::from_be(0xC0A80A2Au);
    constexpr uint16_t port = 8080;
    const auto lhs          = frk::net::ip4_addr_port::from_native(addr, port);
    const auto rhs          = frk::net::ip4_addr_port::from_native(addr, port);

    // WHEN
    const auto eq = lhs == rhs;

    // THEN
    DOCTEST_CHECK(eq);
}

DOCTEST_TEST_CASE("net::ip4_addr_port::operator<=>")
{
    // GIVEN
    constexpr frk::net::ip4_addr::integer_type addr =
        frk::net::from_be(0xC0A80A2Au);
    const auto lhs = frk::net::ip4_addr_port::from_native(addr, 8080);
    const auto rhs = frk::net::ip4_addr_port::from_native(addr, 8081);

    // WHEN
    const auto ord = lhs <=> rhs;

    // THEN
    DOCTEST_CHECK(ord == std::strong_ordering::less);
}

DOCTEST_TEST_CASE("std::formatter<net::ip4_addr_port>")
{
    // GIVEN
    constexpr unsigned char addr[]  = {192, 168, 10, 42};
    constexpr auto port             = frk::net::to_be(uint16_t(8080));
    constexpr std::string_view repr = "192.168.10.42:8080";
    const auto addr_port =
        frk::net::ip4_addr_port::from_be(std::span{addr}, port);

    // WHEN
    const auto str = std::format("{}", addr_port);

    // THEN
    DOCTEST_CHECK_EQ(str, repr);
}

DOCTEST_TEST_CASE("net::ip6_addr_port::from_be(integer)")
{
    // GIVEN
    using int_type  = frk::net::ip6_addr::integer_type;
    const auto addr = (int_type(0x20010DB800000000ull) << 64) |
                      int_type(0x0000000000000001ull);
    constexpr auto port = frk::net::to_be(uint16_t(8080));

    // WHEN
    const auto addr_port = frk::net::ip6_addr_port::from_be(addr, port);

    // THEN
    DOCTEST_CHECK(addr_port.addr_.to_be_integer() == addr);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("net::ip6_addr_port::from_be(span)")
{
    // GIVEN
    constexpr unsigned char addr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    constexpr auto port = frk::net::to_be(uint16_t(8080));

    // WHEN
    const auto addr_port =
        frk::net::ip6_addr_port::from_be(std::span{addr}, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[0], 0x20);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[1], 0x01);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[2], 0x0D);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[3], 0xB8);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[4], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[5], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[6], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[7], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[8], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[9], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[10], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[11], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[12], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[13], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[14], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[15], 0x01);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("net::ip6_addr_port::from_be(in6_addr)")
{
    // GIVEN
    constexpr unsigned char repr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    constexpr auto port = frk::net::to_be(uint16_t(8080));
    const auto addr     = std::bit_cast<in6_addr>(repr);

    // WHEN
    const auto addr_port = frk::net::ip6_addr_port::from_be(addr, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[0], 0x20);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[1], 0x01);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[2], 0x0D);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[3], 0xB8);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[4], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[5], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[6], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[7], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[8], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[9], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[10], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[11], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[12], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[13], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[14], 0x00);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[15], 0x01);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("net::ip6_addr_port::from_native")
{
    // GIVEN
    using int_type = frk::net::ip6_addr::integer_type;
    const auto addr =
        frk::net::from_be((int_type(0x20010DB800000000ull) << 64) |
                          int_type(0x0000000000000001ull));
    constexpr uint16_t port = 8080;

    // WHEN
    const auto addr_port = frk::net::ip6_addr_port::from_native(addr, port);

    // THEN
    DOCTEST_CHECK(addr_port.addr_.to_native_integer() == addr);
    DOCTEST_CHECK_EQ(frk::net::from_be(addr_port.port_), port);
}

DOCTEST_TEST_CASE("net::ip6_addr_port::to_sockaddr_in")
{
    // GIVEN
    using int_type = frk::net::ip6_addr::integer_type;
    const auto addr =
        frk::net::from_be((int_type(0x20010DB800000000ull) << 64) |
                          int_type(0x0000000000000001ull));
    constexpr uint16_t port = 8080;
    const auto addr_port    = frk::net::ip6_addr_port::from_native(addr, port);

    // WHEN
    const auto saddr   = addr_port.to_sockaddr_in();
    const auto in_addr = addr_port.addr_.to_in_addr();

    // THEN
    DOCTEST_CHECK_EQ(saddr.sin6_family, AF_INET6);
    DOCTEST_CHECK_EQ(frk::net::from_be(saddr.sin6_port), port);
    DOCTEST_CHECK_EQ(saddr.sin6_flowinfo, 0u);
    DOCTEST_CHECK_EQ(frk::algo::mem_compare(saddr.sin6_addr, in_addr), 0);
    DOCTEST_CHECK_EQ(saddr.sin6_scope_id, 0u);
}

DOCTEST_TEST_CASE("net::ip6_addr_port::operator==")
{
    // GIVEN
    using int_type = frk::net::ip6_addr::integer_type;
    const auto addr =
        frk::net::from_be((int_type(0x20010DB800000000ull) << 64) |
                          int_type(0x0000000000000001ull));
    constexpr uint16_t port = 8080;
    const auto lhs          = frk::net::ip6_addr_port::from_native(addr, port);
    const auto rhs          = frk::net::ip6_addr_port::from_native(addr, port);

    // WHEN
    const auto eq = lhs == rhs;

    // THEN
    DOCTEST_CHECK(eq);
}

DOCTEST_TEST_CASE("net::ip6_addr_port::operator<=>")
{
    // GIVEN
    using int_type = frk::net::ip6_addr::integer_type;
    const auto addr =
        frk::net::from_be((int_type(0x20010DB800000000ull) << 64) |
                          int_type(0x0000000000000001ull));
    const auto lhs = frk::net::ip6_addr_port::from_native(addr, 8080);
    const auto rhs = frk::net::ip6_addr_port::from_native(addr, 8081);

    // WHEN
    const auto ord = lhs <=> rhs;

    // THEN
    DOCTEST_CHECK(ord == std::strong_ordering::less);
}

DOCTEST_TEST_CASE("std::formatter<net::ip6_addr_port>")
{
    // GIVEN
    constexpr unsigned char addr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    constexpr auto port             = frk::net::to_be(uint16_t(8080));
    constexpr std::string_view repr = "2001:db8::1:8080";
    const auto addr_port =
        frk::net::ip6_addr_port::from_be(std::span{addr}, port);

    // WHEN
    const auto str = std::format("{}", addr_port);

    // THEN
    DOCTEST_CHECK_EQ(str, repr);
}

DOCTEST_TEST_SUITE_END();
