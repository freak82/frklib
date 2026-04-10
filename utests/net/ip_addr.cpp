#include "frk/net/compare.hpp"
#include "frk/net/ip_addr.hpp"

DOCTEST_TEST_SUITE_BEGIN("net/ip_addr");

DOCTEST_TEST_CASE("net::ip4_addr::from_network_order(integer)")
{
    // GIVEN
    constexpr net::ip4_addr::integer_type repr = 0x01020304u;

    // WHEN
    const auto addr = net::ip4_addr::from_network_order(repr);

    // THEN
    DOCTEST_CHECK_EQ(addr.to_network_integer(), repr);
}

DOCTEST_TEST_CASE("net::ip4_addr::from_network_order(span)")
{
    // GIVEN
    constexpr unsigned char repr[] = {0x0A, 0x00, 0x00, 0x01};

    // WHEN
    const auto addr = net::ip4_addr::from_network_order(repr);

    // THEN
    DOCTEST_CHECK_EQ(addr.bytes_[0], 0x0A);
    DOCTEST_CHECK_EQ(addr.bytes_[1], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[2], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[3], 0x01);
}

DOCTEST_TEST_CASE("net::ip4_addr::from_network_order(in_addr)")
{
    // GIVEN
    constexpr unsigned char repr[] = {0x01, 0x02, 0x03, 0x04};
    constexpr auto tmp_addr        = std::bit_cast<in_addr>(repr);

    // WHEN
    const auto addr = net::ip4_addr::from_network_order(tmp_addr);

    // THEN
    DOCTEST_CHECK_EQ(addr.bytes_[0], 0x01);
    DOCTEST_CHECK_EQ(addr.bytes_[1], 0x02);
    DOCTEST_CHECK_EQ(addr.bytes_[2], 0x03);
    DOCTEST_CHECK_EQ(addr.bytes_[3], 0x04);
}

DOCTEST_TEST_CASE("net::ip4_addr::from_native_order")
{
    // GIVEN
    constexpr net::ip4_addr::integer_type repr =
        ben::big_to_native(0x01020304u);

    // WHEN
    const auto addr = net::ip4_addr::from_native_order(repr);

    // THEN
    DOCTEST_CHECK_EQ(addr.to_native_integer(), repr);
}

DOCTEST_TEST_CASE("net::ip4_addr::from_string(valid)")
{
    // GIVEN
    constexpr std::string_view repr = "192.168.10.42";

    // WHEN
    const auto addr = net::ip4_addr::from_string(repr);

    // THEN
    DOCTEST_CHECK_EQ(addr.value().bytes_[0], 192);
    DOCTEST_CHECK_EQ(addr.value().bytes_[1], 168);
    DOCTEST_CHECK_EQ(addr.value().bytes_[2], 10);
    DOCTEST_CHECK_EQ(addr.value().bytes_[3], 42);
}

DOCTEST_TEST_CASE("net::ip4_addr::from_string(invalid)")
{
    // GIVEN
    constexpr std::string_view repr = "300.168.10.42";

    // WHEN
    const auto addr = net::ip4_addr::from_string(repr);

    // THEN
    DOCTEST_CHECK(!addr.has_value());
}

DOCTEST_TEST_CASE("net::ip4_addr::from_string(too_long)")
{
    // GIVEN
    constexpr std::string_view repr = "255.255.255.2550";

    // WHEN
    const auto addr = net::ip4_addr::from_string(repr);

    // THEN
    DOCTEST_CHECK(!addr.has_value());
}

DOCTEST_TEST_CASE("net::ip4_addr::to_network_integer")
{
    // GIVEN
    constexpr net::ip4_addr::integer_type repr = 0x0A000001u;
    const auto addr = net::ip4_addr::from_network_order(repr);

    // WHEN
    const auto integer = addr.to_network_integer();

    // THEN
    DOCTEST_CHECK_EQ(integer, repr);
}

DOCTEST_TEST_CASE("net::ip4_addr::to_native_integer")
{
    // GIVEN
    constexpr net::ip4_addr::integer_type repr =
        ben::big_to_native(0x0A000001u);
    const auto addr = net::ip4_addr::from_native_order(repr);

    // WHEN
    const auto integer = addr.to_native_integer();

    // THEN
    DOCTEST_CHECK_EQ(integer, repr);
}

DOCTEST_TEST_CASE("net::ip4_addr::to_in_addr")
{
    // GIVEN
    constexpr unsigned char repr[] = {0x7F, 0x00, 0x00, 0x01};
    const auto addr                = net::ip4_addr::from_network_order(repr);

    // WHEN
    const auto in_addr = addr.to_in_addr();

    // THEN
    DOCTEST_CHECK_EQ(net::mem_compare(in_addr, repr), 0);
}

DOCTEST_TEST_CASE("net::ip4_addr::mask")
{
    // GIVEN
    const auto addr = net::ip4_addr::from_string("192.168.10.42");

    // WHEN
    const auto masked = addr.value().mask(net::prefix_len{24});

    // THEN
    DOCTEST_CHECK_EQ(masked.bytes_[0], 192);
    DOCTEST_CHECK_EQ(masked.bytes_[1], 168);
    DOCTEST_CHECK_EQ(masked.bytes_[2], 10);
    DOCTEST_CHECK_EQ(masked.bytes_[3], 0);
}

DOCTEST_TEST_CASE("net::ip4_addr::operator==")
{
    // GIVEN
    const auto lhs = net::ip4_addr::from_string("192.168.10.42");
    const auto rhs = net::ip4_addr::from_string("192.168.10.42");

    // WHEN
    const auto eq = lhs.value() == rhs.value();

    // THEN
    DOCTEST_CHECK(eq);
}

DOCTEST_TEST_CASE("net::ip4_addr::operator<=>")
{
    // GIVEN
    const auto lhs = net::ip4_addr::from_string("192.168.10.41");
    const auto rhs = net::ip4_addr::from_string("192.168.10.42");

    // WHEN
    const auto ord = lhs.value() <=> rhs.value();

    // THEN
    DOCTEST_CHECK(ord == std::strong_ordering::less);
}

DOCTEST_TEST_CASE("fmt::formatter<net::ip4_addr>")
{
    // GIVEN
    constexpr std::string_view repr = "192.168.10.42";
    const auto addr                 = net::ip4_addr::from_string(repr);

    // WHEN
    const auto str = fmt::format("{}", addr.value());

    // THEN
    DOCTEST_CHECK_EQ(str, repr);
}

DOCTEST_TEST_CASE("net::ip6_addr::from_network_order(integer)")
{
    // GIVEN
    using int_type  = net::ip6_addr::integer_type;
    const auto repr = (int_type(0x20010DB800000000ull) << 64) |
                      int_type(0x0000000000000001ull);

    // WHEN
    const auto addr = net::ip6_addr::from_network_order(repr);

    // THEN
    DOCTEST_CHECK(addr.to_network_integer() == repr);
}

DOCTEST_TEST_CASE("net::ip6_addr::from_network_order(span)")
{
    // GIVEN
    constexpr unsigned char repr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };

    // WHEN
    const auto addr = net::ip6_addr::from_network_order(repr);

    // THEN
    DOCTEST_CHECK_EQ(addr.bytes_[0], 0x20);
    DOCTEST_CHECK_EQ(addr.bytes_[1], 0x01);
    DOCTEST_CHECK_EQ(addr.bytes_[2], 0x0D);
    DOCTEST_CHECK_EQ(addr.bytes_[3], 0xB8);
    DOCTEST_CHECK_EQ(addr.bytes_[4], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[5], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[6], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[7], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[8], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[9], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[10], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[11], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[12], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[13], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[14], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[15], 0x01);
}

DOCTEST_TEST_CASE("net::ip6_addr::from_network_order(in6_addr)")
{
    // GIVEN
    constexpr unsigned char repr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    const auto tmp_addr = std::bit_cast<in6_addr>(repr);

    // WHEN
    const auto addr = net::ip6_addr::from_network_order(tmp_addr);

    // THEN
    DOCTEST_CHECK_EQ(addr.bytes_[0], 0x20);
    DOCTEST_CHECK_EQ(addr.bytes_[1], 0x01);
    DOCTEST_CHECK_EQ(addr.bytes_[2], 0x0D);
    DOCTEST_CHECK_EQ(addr.bytes_[3], 0xB8);
    DOCTEST_CHECK_EQ(addr.bytes_[4], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[5], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[6], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[7], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[8], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[9], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[10], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[11], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[12], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[13], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[14], 0x00);
    DOCTEST_CHECK_EQ(addr.bytes_[15], 0x01);
}

DOCTEST_TEST_CASE("net::ip6_addr::from_native_order")
{
    // GIVEN
    using int_type = net::ip6_addr::integer_type;
    const auto repr =
        ben::big_to_native((int_type(0x20010DB800000000ull) << 64) |
                           int_type(0x0000000000000001ull));

    // WHEN
    const auto addr = net::ip6_addr::from_native_order(repr);

    // THEN
    DOCTEST_CHECK(addr.to_native_integer() == repr);
}

DOCTEST_TEST_CASE("net::ip6_addr::from_string(valid)")
{
    // GIVEN
    constexpr std::string_view repr = "2001:db8::1";

    // WHEN
    const auto addr = net::ip6_addr::from_string(repr);

    // THEN
    DOCTEST_CHECK_EQ(addr.value().bytes_[0], 0x20);
    DOCTEST_CHECK_EQ(addr.value().bytes_[1], 0x01);
    DOCTEST_CHECK_EQ(addr.value().bytes_[2], 0x0D);
    DOCTEST_CHECK_EQ(addr.value().bytes_[3], 0xB8);
    DOCTEST_CHECK_EQ(addr.value().bytes_[4], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[5], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[6], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[7], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[8], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[9], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[10], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[11], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[12], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[13], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[14], 0x00);
    DOCTEST_CHECK_EQ(addr.value().bytes_[15], 0x01);
}

DOCTEST_TEST_CASE("net::ip6_addr::from_string(invalid)")
{
    // GIVEN
    constexpr std::string_view repr = "2001:db8::zz";

    // WHEN
    const auto addr = net::ip6_addr::from_string(repr);

    // THEN
    DOCTEST_CHECK(!addr.has_value());
}

DOCTEST_TEST_CASE("net::ip6_addr::from_string(too_long)")
{
    // GIVEN
    constexpr std::string_view repr =
        "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.2550";

    // WHEN
    const auto addr = net::ip6_addr::from_string(repr);

    // THEN
    DOCTEST_CHECK(!addr.has_value());
}

DOCTEST_TEST_CASE("net::ip6_addr::to_network_integer")
{
    // GIVEN
    using int_type  = net::ip6_addr::integer_type;
    const auto repr = (int_type(0x20010DB800000000ull) << 64) |
                      int_type(0x0000000000000001ull);
    const auto addr = net::ip6_addr::from_network_order(repr);

    // WHEN
    const auto integer = addr.to_network_integer();

    // THEN
    DOCTEST_CHECK(integer == repr);
}

DOCTEST_TEST_CASE("net::ip6_addr::to_native_integer")
{
    // GIVEN
    using int_type = net::ip6_addr::integer_type;
    const auto repr =
        ben::big_to_native((int_type(0x20010DB800000000ull) << 64) |
                           int_type(0x0000000000000001ull));
    const auto addr = net::ip6_addr::from_native_order(repr);

    // WHEN
    const auto integer = addr.to_native_integer();

    // THEN
    DOCTEST_CHECK(integer == repr);
}

DOCTEST_TEST_CASE("net::ip6_addr::to_in_addr")
{
    // GIVEN
    constexpr unsigned char repr[] = {
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    const auto addr = net::ip6_addr::from_network_order(repr);

    // WHEN
    const auto in_addr = addr.to_in_addr();

    // THEN
    DOCTEST_CHECK_EQ(net::mem_compare(in_addr, repr), 0);
}

DOCTEST_TEST_CASE("net::ip6_addr::mask")
{
    // GIVEN
    const auto addr = net::ip6_addr::from_string("2001:db8::1");

    // WHEN
    const auto masked = addr.value().mask(net::prefix_len{64});

    // THEN
    DOCTEST_CHECK_EQ(masked.bytes_[0], 0x20);
    DOCTEST_CHECK_EQ(masked.bytes_[1], 0x01);
    DOCTEST_CHECK_EQ(masked.bytes_[2], 0x0D);
    DOCTEST_CHECK_EQ(masked.bytes_[3], 0xB8);
    DOCTEST_CHECK_EQ(masked.bytes_[4], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[5], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[6], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[7], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[8], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[9], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[10], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[11], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[12], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[13], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[14], 0x00);
    DOCTEST_CHECK_EQ(masked.bytes_[15], 0x00);
}

DOCTEST_TEST_CASE("net::ip6_addr::operator==")
{
    // GIVEN
    const auto lhs = net::ip6_addr::from_string("2001:db8::1");
    const auto rhs = net::ip6_addr::from_string("2001:db8::1");

    // WHEN
    const auto eq = lhs.value() == rhs.value();

    // THEN
    DOCTEST_CHECK(eq);
}

DOCTEST_TEST_CASE("net::ip6_addr::operator<=>")
{
    // GIVEN
    const auto lhs = net::ip6_addr::from_string("2001:db8::1");
    const auto rhs = net::ip6_addr::from_string("2001:db8::2");

    // WHEN
    const auto ord = lhs.value() <=> rhs.value();

    // THEN
    DOCTEST_CHECK(ord == std::strong_ordering::less);
}

DOCTEST_TEST_CASE("fmt::formatter<net::ip6_addr>")
{
    // GIVEN
    constexpr std::string_view repr = "2001:db8::1";
    const auto addr                 = net::ip6_addr::from_string(repr);

    // WHEN
    const auto str = fmt::format("{}", addr.value());

    // THEN
    DOCTEST_CHECK_EQ(str, repr);
}

DOCTEST_TEST_SUITE_END();
