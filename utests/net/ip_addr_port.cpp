#include "app/put/algorithm.hpp"
#include "app/put/compare.hpp"
#include "app/put/ip_addr_port.hpp"

DOCTEST_TEST_SUITE_BEGIN("put/ip_addr_port");

DOCTEST_TEST_CASE("put::ip4_addr_port::from_network_order(integer)")
{
    // GIVEN
    constexpr put::ip4_addr::integer_type addr = 0x0A000001u;
    constexpr auto port = ben::native_to_big(uint16_t(8080));

    // WHEN
    const auto addr_port = put::ip4_addr_port::from_network_order(addr, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.to_network_integer(), addr);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("put::ip4_addr_port::from_network_order(span)")
{
    // GIVEN
    constexpr unsigned char addr[] = {192, 168, 10, 42};
    constexpr auto port            = ben::native_to_big(uint16_t(8080));

    // WHEN
    const auto addr_port =
        put::ip4_addr_port::from_network_order(std::span{addr}, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[0], 192);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[1], 168);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[2], 10);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[3], 42);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("put::ip4_addr_port::from_network_order(in_addr)")
{
    // GIVEN
    constexpr unsigned char repr[] = {192, 168, 10, 42};
    constexpr auto addr            = std::bit_cast<in_addr>(repr);
    constexpr auto port            = ben::native_to_big(uint16_t(8080));

    // WHEN
    const auto addr_port = put::ip4_addr_port::from_network_order(addr, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[0], 192);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[1], 168);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[2], 10);
    DOCTEST_CHECK_EQ(addr_port.addr_.bytes_[3], 42);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("put::ip4_addr_port::from_native_order")
{
    // GIVEN
    constexpr put::ip4_addr::integer_type addr =
        ben::big_to_native(0x0A000001u);
    constexpr uint16_t port = 8080;

    // WHEN
    const auto addr_port = put::ip4_addr_port::from_native_order(addr, port);

    // THEN
    DOCTEST_CHECK_EQ(addr_port.addr_.to_native_integer(), addr);
    DOCTEST_CHECK_EQ(ben::big_to_native(addr_port.port_), port);
}

DOCTEST_TEST_CASE("put::ip4_addr_port::to_sockaddr_in")
{
    // GIVEN
    constexpr put::ip4_addr::integer_type addr = 0xC0A80A2Au;
    constexpr uint16_t port                    = 8080;
    const auto addr_port = put::ip4_addr_port::from_native_order(addr, port);

    // WHEN
    const auto saddr = addr_port.to_sockaddr_in();

    // THEN
    DOCTEST_CHECK_EQ(saddr.sin_family, AF_INET);
    DOCTEST_CHECK_EQ(ben::big_to_native(saddr.sin_port), port);
    DOCTEST_CHECK_EQ(saddr.sin_addr.s_addr,
                     addr_port.addr_.to_in_addr().s_addr);
    DOCTEST_CHECK(put::all_of(saddr.sin_zero, 0));
}

DOCTEST_TEST_CASE("put::ip4_addr_port::operator==")
{
    // GIVEN
    constexpr put::ip4_addr::integer_type addr = 0xC0A80A2Au;
    constexpr uint16_t port                    = 8080;
    const auto lhs = put::ip4_addr_port::from_native_order(addr, port);
    const auto rhs = put::ip4_addr_port::from_native_order(addr, port);

    // WHEN
    const auto eq = lhs == rhs;

    // THEN
    DOCTEST_CHECK(eq);
}

DOCTEST_TEST_CASE("put::ip4_addr_port::operator<=>")
{
    // GIVEN
    constexpr put::ip4_addr::integer_type addr = 0xC0A80A2Au;
    const auto lhs = put::ip4_addr_port::from_native_order(addr, 8080);
    const auto rhs = put::ip4_addr_port::from_native_order(addr, 8081);

    // WHEN
    const auto ord = lhs <=> rhs;

    // THEN
    DOCTEST_CHECK(ord == std::strong_ordering::less);
}

DOCTEST_TEST_CASE("fmt::formatter<put::ip4_addr_port>")
{
    // GIVEN
    constexpr put::ip4_addr::integer_type addr = 0xC0A80A2Au;
    constexpr uint16_t port                    = 8080;
    constexpr std::string_view repr            = "192.168.10.42:8080";
    const auto addr_port = put::ip4_addr_port::from_native_order(addr, port);

    // WHEN
    const auto str = fmt::format("{}", addr_port);

    // THEN
    DOCTEST_CHECK_EQ(str, repr);
}

DOCTEST_TEST_CASE("put::ip6_addr_port::from_network_order(integer)")
{
    // GIVEN
    using int_type  = put::ip6_addr::integer_type;
    const auto addr = (int_type(0x20010DB800000000ull) << 64) |
                      int_type(0x0000000000000001ull);
    constexpr auto port = ben::native_to_big(uint16_t(8080));

    // WHEN
    const auto addr_port = put::ip6_addr_port::from_network_order(addr, port);

    // THEN
    DOCTEST_CHECK(addr_port.addr_.to_network_integer() == addr);
    DOCTEST_CHECK_EQ(addr_port.port_, port);
}

DOCTEST_TEST_CASE("put::ip6_addr_port::from_network_order(span)")
{
    // GIVEN
    constexpr unsigned char addr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    constexpr auto port = ben::native_to_big(uint16_t(8080));

    // WHEN
    const auto addr_port =
        put::ip6_addr_port::from_network_order(std::span{addr}, port);

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

DOCTEST_TEST_CASE("put::ip6_addr_port::from_network_order(in6_addr)")
{
    // GIVEN
    constexpr unsigned char repr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    constexpr auto port = ben::native_to_big(uint16_t(8080));
    const auto addr     = std::bit_cast<in6_addr>(repr);

    // WHEN
    const auto addr_port = put::ip6_addr_port::from_network_order(addr, port);

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

DOCTEST_TEST_CASE("put::ip6_addr_port::from_native_order")
{
    // GIVEN
    using int_type = put::ip6_addr::integer_type;
    const auto addr =
        ben::big_to_native((int_type(0x20010DB800000000ull) << 64) |
                           int_type(0x0000000000000001ull));
    constexpr uint16_t port = 8080;

    // WHEN
    const auto addr_port = put::ip6_addr_port::from_native_order(addr, port);

    // THEN
    DOCTEST_CHECK(addr_port.addr_.to_native_integer() == addr);
    DOCTEST_CHECK_EQ(ben::big_to_native(addr_port.port_), port);
}

DOCTEST_TEST_CASE("put::ip6_addr_port::to_sockaddr_in")
{
    // GIVEN
    using int_type = put::ip6_addr::integer_type;
    const auto addr =
        ben::big_to_native((int_type(0x20010DB800000000ull) << 64) |
                           int_type(0x0000000000000001ull));
    constexpr uint16_t port = 8080;
    const auto addr_port    = put::ip6_addr_port::from_native_order(addr, port);

    // WHEN
    const auto saddr   = addr_port.to_sockaddr_in();
    const auto in_addr = addr_port.addr_.to_in_addr();

    // THEN
    DOCTEST_CHECK_EQ(saddr.sin6_family, AF_INET6);
    DOCTEST_CHECK_EQ(ben::big_to_native(saddr.sin6_port), port);
    DOCTEST_CHECK_EQ(saddr.sin6_flowinfo, 0u);
    DOCTEST_CHECK_EQ(put::mem_compare(saddr.sin6_addr, in_addr), 0);
    DOCTEST_CHECK_EQ(saddr.sin6_scope_id, 0u);
}

DOCTEST_TEST_CASE("put::ip6_addr_port::operator==")
{
    // GIVEN
    using int_type = put::ip6_addr::integer_type;
    const auto addr =
        ben::big_to_native((int_type(0x20010DB800000000ull) << 64) |
                           int_type(0x0000000000000001ull));
    constexpr uint16_t port = 8080;
    const auto lhs          = put::ip6_addr_port::from_native_order(addr, port);
    const auto rhs          = put::ip6_addr_port::from_native_order(addr, port);

    // WHEN
    const auto eq = (lhs == rhs);

    // THEN
    DOCTEST_CHECK(eq);
}

DOCTEST_TEST_CASE("put::ip6_addr_port::operator<=>")
{
    // GIVEN
    using int_type = put::ip6_addr::integer_type;
    const auto addr =
        ben::big_to_native((int_type(0x20010DB800000000ull) << 64) |
                           int_type(0x0000000000000001ull));
    const auto lhs = put::ip6_addr_port::from_native_order(addr, 8080);
    const auto rhs = put::ip6_addr_port::from_native_order(addr, 8081);

    // WHEN
    const auto ord = (lhs <=> rhs);

    // THEN
    DOCTEST_CHECK(ord == std::strong_ordering::less);
}

DOCTEST_TEST_CASE("fmt::formatter<put::ip6_addr_port>")
{
    // GIVEN
    constexpr unsigned char addr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    constexpr auto port             = ben::native_to_big(uint16_t(8080));
    constexpr std::string_view repr = "2001:db8::1:8080";
    const auto addr_port =
        put::ip6_addr_port::from_network_order(std::span{addr}, port);

    // WHEN
    const auto str = fmt::format("{}", addr_port);

    // THEN
    DOCTEST_CHECK_EQ(str, repr);
}

DOCTEST_TEST_SUITE_END();
