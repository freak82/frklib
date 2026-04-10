#pragma once

#include "put/ip_addr.hpp"

// TODO: Unit tests
namespace put
{

template <typename AddrRepr, typename Addr>
concept ReprFromNetworkOrder = requires(AddrRepr repr) {
    { Addr::from_network_order(repr) } -> std::same_as<Addr>;
};

template <typename AddrRepr, typename Addr>
concept ReprFromNativeOrder = requires(AddrRepr repr) {
    { Addr::from_native_order(repr) } -> std::same_as<Addr>;
};

struct ip4_addr_port
{
    using addr_type = ip4_addr;

    addr_type addr_;
    __be16 port_;

    template <ReprFromNetworkOrder<addr_type> AddrRepr>
    [[nodiscard]] static constexpr ip4_addr_port
    from_network_order(AddrRepr addr, __be16 port) noexcept
    {
        return {
            .addr_ = addr_type::from_network_order(addr),
            .port_ = port,
        };
    }

    template <ReprFromNativeOrder<addr_type> AddrRepr>
    [[nodiscard]] static constexpr ip4_addr_port
    from_native_order(AddrRepr addr, uint16_t port) noexcept
    {
        return {
            .addr_ = addr_type::from_native_order(addr),
            .port_ = ben::native_to_big(port),
        };
    }

    [[nodiscard]] constexpr sockaddr_in to_sockaddr_in() const noexcept
    {
        return {
            .sin_family = AF_INET,
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

    template <ReprFromNetworkOrder<addr_type> AddrRepr>
    [[nodiscard]] static constexpr ip6_addr_port
    from_network_order(AddrRepr addr, __be16 port) noexcept
    {
        return {
            .addr_ = addr_type::from_network_order(addr),
            .port_ = port,
        };
    }

    template <ReprFromNativeOrder<addr_type> AddrRepr>
    [[nodiscard]] static constexpr ip6_addr_port
    from_native_order(AddrRepr addr, uint16_t port) noexcept
    {
        return {
            .addr_ = addr_type::from_native_order(addr),
            .port_ = ben::native_to_big(port),
        };
    }

    [[nodiscard]] constexpr sockaddr_in6 to_sockaddr_in() const noexcept
    {
        return {
            .sin6_family   = AF_INET6,
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

} // namespace put
////////////////////////////////////////////////////////////////////////////////
// The formatters are done in this way so that the parsing functionality of the
// base formatter can be reused and the output to be able to be aligned, etc.
template <>
struct fmt::formatter<put::ip4_addr_port> : fmt::formatter<std::string_view>
{
    auto format(const auto& arg, auto& ctx) const noexcept
    {
        using base_type = fmt::formatter<std::string_view>;
        char buf[INET_ADDRSTRLEN + 8];
        auto [out, _] = fmt::format_to_n(buf, sizeof(buf), "{}:{}", arg.addr_,
                                         ben::big_to_native(arg.port_));
        return base_type::format(
            std::string_view(buf, static_cast<size_t>(out - buf)), ctx);
    }
};

template <>
struct fmt::formatter<put::ip6_addr_port> : fmt::formatter<std::string_view>
{
    auto format(const auto& arg, auto& ctx) const noexcept
    {
        using base_type = fmt::formatter<std::string_view>;
        char buf[INET6_ADDRSTRLEN + 8];
        auto [out, _] = fmt::format_to_n(buf, sizeof(buf), "{}:{}", arg.addr_,
                                         ben::big_to_native(arg.port_));
        return base_type::format(
            std::string_view(buf, static_cast<size_t>(out - buf)), ctx);
    }
};
