#pragma once

namespace freak::par
{

struct cpu
{
    // TODO: Additional functionality to verify such `constexpr` constants at
    // startup should be provided.
    static constexpr auto cache_line_size = 64uz;

    static inline void pause() noexcept { __builtin_ia32_pause(); }
};

} // namespace freak::par
