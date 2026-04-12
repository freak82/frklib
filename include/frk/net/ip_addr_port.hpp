#pragma once

#include "frk/net/endian.hpp"
#include "frk/net/ip_addr.hpp"

#include <format>
#include <type_traits>

namespace frk::net
{

template <typename AddrRepr, typename Addr>
concept repr_from_be = requires(AddrRepr repr) {
    { Addr::from_be(repr) } -> std::same_as<Addr>;
};

template <typename AddrRepr, typename Addr>
concept repr_from_native = requires(AddrRepr repr) {
    { Addr::from_native(repr) } -> std::same_as<Addr>;
};

struct ip4_addr_port
{
    using addr_type = ip4_addr;

    addr_type addr_;
    __be16 port_;

    template <repr_from_be<addr_type> AddrRepr>
    [[nodiscard]] static constexpr ip4_addr_port from_be(AddrRepr addr,
                                                         __be16 port) noexcept
    {
        return {
            .addr_ = addr_type::from_be(addr),
            .port_ = port,
        };
    }

    template <repr_from_native<addr_type> AddrRepr>
    [[nodiscard]] static constexpr ip4_addr_port
    from_native(AddrRepr addr, uint16_t port) noexcept
    {
        return {
            .addr_ = addr_type::from_native(addr),
            .port_ = frk::net::to_be(port),
        };
    }

    [[nodiscard]] constexpr sockaddr_in to_sockaddr_in() const noexcept
    {
        return {
            .sin_family = addr_type::family,
            .sin_port   = port_,
            .sin_addr   = addr_.to_in_addr(),
            .sin_zero   = {},
        };
    }

    constexpr auto operator<=>(const ip4_addr_port&) const noexcept = default;
    constexpr bool operator==(ip4_addr_port rhs) const noexcept
    {
        return (addr_ == rhs.addr_) && (port_ == rhs.port_);
    }
};

struct ip6_addr_port
{
    using addr_type = ip6_addr;

    ip6_addr addr_;
    __be16 port_;

    template <repr_from_be<addr_type> AddrRepr>
    [[nodiscard]] static constexpr ip6_addr_port from_be(AddrRepr addr,
                                                         __be16 port) noexcept
    {
        return {
            .addr_ = addr_type::from_be(addr),
            .port_ = port,
        };
    }

    template <repr_from_native<addr_type> AddrRepr>
    [[nodiscard]] static constexpr ip6_addr_port
    from_native(AddrRepr addr, uint16_t port) noexcept
    {
        return {
            .addr_ = addr_type::from_native(addr),
            .port_ = frk::net::to_be(port),
        };
    }

    [[nodiscard]] constexpr sockaddr_in6 to_sockaddr_in() const noexcept
    {
        return {
            .sin6_family   = addr_type::family,
            .sin6_port     = port_,
            .sin6_flowinfo = {},
            .sin6_addr     = addr_.to_in_addr(),
            .sin6_scope_id = {},
        };
    }

    constexpr auto operator<=>(const ip6_addr_port&) const noexcept = default;
    constexpr bool operator==(const ip6_addr_port& rhs) const noexcept
    {
        return (addr_ == rhs.addr_) && (port_ == rhs.port_);
    }
};

} // namespace frk::net
////////////////////////////////////////////////////////////////////////////////
// The formatter is done in this way so that the parsing functionality of the
// base formatter can be reused and the output to be able to be aligned, etc.
template <typename AddrPort>
    requires(std::is_same_v<AddrPort, frk::net::ip4_addr_port> ||
             std::is_same_v<AddrPort, frk::net::ip6_addr_port>)
struct std::formatter<AddrPort> : std::formatter<std::string_view>
{
    auto format(const AddrPort& arg, auto& ctx) const noexcept
    {
        using base_type = std::formatter<std::string_view>;
        using addr_type = AddrPort::addr_type;
        char buf[addr_type::str_len + 8]; // 6 bytes for the port is enough
        auto [out, _] = std::format_to_n(buf, sizeof(buf), "{}:{}", arg.addr_,
                                         frk::net::from_be(arg.port_));
        return base_type::format(
            std::string_view(buf, static_cast<size_t>(out - buf)), ctx);
    }
};
