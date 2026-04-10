#pragma once

#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>

#include <bit>
#include <span>

namespace frk::net
{
// A helper class for simulating named arguments.
struct prefix_len
{
    uint8_t nbits;

    constexpr auto operator<=>(const prefix_len&) const noexcept = default;
};
// The types are intentionally left trivial with static functions for
// construction. This generates much better assembly than when
// boost::asio::ip::address_v4/address_v6 is used.
// The IPv4 address is intentionally stored as bytes instead of 32 bit integer
// so that the `ip4_addr` type has smaller alignment requirements and remove the
// possibility of wholes
struct ip4_addr
{
    using integer_type = uint32_t;

    static constexpr uint8_t cnt_bits  = 32;
    static constexpr uint8_t cnt_bytes = 4;

    unsigned char bytes_[4];

    [[nodiscard]] static constexpr ip4_addr
    from_network_order(integer_type v) noexcept
    {
        return std::bit_cast<ip4_addr>(v);
    }
    [[nodiscard]] static constexpr ip4_addr
    from_network_order(std::span<const unsigned char, 4> v) noexcept
    {
        ip4_addr ret;
        ::memcpy(ret.bytes_, v.data(), v.size());
        return ret;
    }
    [[nodiscard]] static constexpr ip4_addr
    from_network_order(in_addr v) noexcept
    {
        return std::bit_cast<ip4_addr>(v);
    }
    [[nodiscard]] static constexpr ip4_addr
    from_native_order(integer_type v) noexcept
    {
        return from_network_order(ben::native_to_big(v));
    }

    [[nodiscard]] static constexpr std::optional<ip4_addr>
    from_string(std::string_view str) noexcept
    {
        // The C API expects NULL terminated string and thus we need the copy.
        char tmp[INET_ADDRSTRLEN];
        if (str.size() >= sizeof(tmp)) return std::nullopt;
        ::memcpy(tmp, str.data(), str.size());
        tmp[str.size()] = '\0';

        ip4_addr ret;
        static_assert(sizeof(ret.bytes_) == sizeof(in_addr));
        if (!::inet_pton(AF_INET, tmp, ret.bytes_)) return std::nullopt;
        return ret;
    }

    [[nodiscard]] constexpr integer_type to_network_integer() const noexcept
    {
        return std::bit_cast<integer_type>(bytes_);
    }
    [[nodiscard]] constexpr integer_type to_native_integer() const noexcept
    {
        return ben::big_to_native(to_network_integer());
    }
    [[nodiscard]] constexpr in_addr to_in_addr() const noexcept
    {
        return std::bit_cast<in_addr>(bytes_);
    }

    [[nodiscard]] constexpr ip4_addr mask(prefix_len p) const noexcept
    {
        const auto mask = (integer_type(1) << (cnt_bits - p.nbits)) - 1;
        return from_native_order(to_native_integer() & ~mask);
    }

    // As of GCC 12.1 the compiler sill generates much worse code if we
    // default these operations.
    constexpr std::strong_ordering operator<=>(ip4_addr rhs) const noexcept
    {
        return (to_network_integer() <=> rhs.to_network_integer());
    }
    constexpr bool operator==(ip4_addr rhs) const noexcept
    {
        return (to_network_integer() == rhs.to_network_integer());
    }
};

struct ip6_addr
{
    // Silence pedantic warning for ISO C++ not supporting __int128
    __extension__ using integer_type = unsigned __int128;

    static constexpr uint8_t cnt_bits  = 128;
    static constexpr uint8_t cnt_bytes = 16;

    unsigned char bytes_[16];

    [[nodiscard]] static constexpr ip6_addr
    from_network_order(integer_type v) noexcept
    {
        return std::bit_cast<ip6_addr>(v);
    }
    [[nodiscard]] static constexpr ip6_addr
    from_network_order(std::span<const unsigned char, 16> v) noexcept
    {
        ip6_addr ret;
        ::memcpy(ret.bytes_, v.data(), v.size());
        return ret;
    }
    [[nodiscard]] static constexpr ip6_addr
    from_network_order(in6_addr v) noexcept
    {
        return std::bit_cast<ip6_addr>(v);
    }
    [[nodiscard]] static constexpr ip6_addr
    from_native_order(integer_type v) noexcept
    {
        return from_network_order(ben::native_to_big(v));
    }

    [[nodiscard]] static constexpr std::optional<ip6_addr>
    from_string(std::string_view str) noexcept
    {
        // The C API expects NULL terminated string and thus we need the copy.
        char tmp[INET6_ADDRSTRLEN];
        if (str.size() >= sizeof(tmp)) return std::nullopt;
        ::memcpy(tmp, str.data(), str.size());
        tmp[str.size()] = '\0';

        ip6_addr ret;
        static_assert(sizeof(ret.bytes_) == sizeof(in6_addr));
        if (!::inet_pton(AF_INET6, tmp, ret.bytes_)) return std::nullopt;
        return ret;
    }

    [[nodiscard]] constexpr integer_type to_network_integer() const noexcept
    {
        return std::bit_cast<integer_type>(bytes_);
    }
    [[nodiscard]] constexpr integer_type to_native_integer() const noexcept
    {
        return ben::big_to_native(to_network_integer());
    }
    [[nodiscard]] constexpr in6_addr to_in_addr() const noexcept
    {
        return std::bit_cast<in6_addr>(bytes_);
    }

    [[nodiscard]] constexpr ip6_addr mask(prefix_len p) const noexcept
    {
        const auto mask = (integer_type(1) << (cnt_bits - p.nbits)) - 1;
        return from_native_order(to_native_integer() & ~mask);
    }

    // As of GCC 12.1 the compiler sill generates much worse code if we
    // default these operations. And this is how much better is the current
    // implementation using std::bit_cast than the one using std::memcmp.
    // cmp_memcmp(ip6_addr, ip6_addr):     # @cmp_memcmp(ip6_addr, ip6_addr)
    //     mov     qword ptr [rsp - 16], rdi
    //     mov     qword ptr [rsp - 8], rsi
    //     mov     qword ptr [rsp - 32], rdx
    //     mov     qword ptr [rsp - 24], rcx
    //     movdqu  xmm0, xmmword ptr [rsp - 16]
    //     movdqu  xmm1, xmmword ptr [rsp - 32]
    //     pcmpeqb xmm1, xmm0
    //     pmovmskb        eax, xmm1
    //     cmp     eax, 65535
    //     sete    al
    //     ret
    // cmp_bitcast(ip6_addr, ip6_addr):    # @cmp_bitcast(ip6_addr, ip6_addr)
    //     xor     rsi, rcx
    //     xor     rdi, rdx
    //     or      rdi, rsi
    //     sete    al
    //     ret
    constexpr std::strong_ordering operator<=>(ip6_addr rhs) const noexcept
    {
        return (to_network_integer() <=> rhs.to_network_integer());
    }
    constexpr bool operator==(ip6_addr rhs) const noexcept
    {
        return (to_network_integer() == rhs.to_network_integer());
    }
};

} // namespace frk::net
////////////////////////////////////////////////////////////////////////////////
// The formatters are done in this way so that the parsing functionality of the
// base formatter can be reused and the output to be able to be aligned, etc.
template <>
struct fmt::formatter<put::ip4_addr> : fmt::formatter<std::string_view>
{
    auto format(const auto& arg, auto& ctx) const noexcept
    {
        using base_type = fmt::formatter<std::string_view>;
        char buf[INET_ADDRSTRLEN];
        auto [out, _] =
            fmt::format_to_n(buf, sizeof(buf), "{}.{}.{}.{}", arg.bytes_[0],
                             arg.bytes_[1], arg.bytes_[2], arg.bytes_[3]);
        return base_type::format(
            std::string_view(buf, static_cast<size_t>(out - buf)), ctx);
    }
};

template <>
struct fmt::formatter<put::ip6_addr> : fmt::formatter<std::string_view>
{
    auto format(const auto& arg, auto& ctx) const noexcept
    {
        using base_type = fmt::formatter<std::string_view>;
        char buff[INET6_ADDRSTRLEN];
        const auto addr = arg.to_in_addr();
        const char* str = ::inet_ntop(AF_INET6, &addr, buff, sizeof(buff));
        return str ? base_type::format(std::string_view(str), ctx) : ctx.out();
    }
};
