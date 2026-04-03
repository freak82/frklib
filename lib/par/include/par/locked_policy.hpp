#pragma once

namespace freak::par
{

struct unique_locked_policy
{
    static constexpr bool const_data = false; // Allows non const data

    template <typename Mutex>
    static void lock(Mutex& m) noexcept(noexcept(m.lock()))
    {
        m.lock();
    }
    template <typename Mutex>
    static void unlock(Mutex& m) noexcept
    {
        m.unlock();
    }
};

struct shared_locked_policy
{
    static constexpr bool const_data = true; // Allows only const data

    template <typename Mutex>
    static void lock(Mutex& m) noexcept(noexcept(m.lock_shared()))
    {
        m.lock_shared();
    }
    template <typename Mutex>
    static void unlock(Mutex& m) noexcept
    {
        m.unlock_shared();
    }
};

} // namespace freak::par
