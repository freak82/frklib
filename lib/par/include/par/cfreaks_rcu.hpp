#pragma once

#include <stdint.h>

#include <atomic>
#include <memory>

#include "par/cpu.hpp"

namespace freak::par
{

// This is the implementation tho phase user space RCU by the so called
// concurrency freaks.
// https://github.com/pramalhe/ConcurrencyFreaks/blob/master/CPP/papers/
// gracesharingurcu/URCUTwoPhase.hpp
//
// TODO: Add allocator support
class cfreaks_rcu
{
    struct alignas(cpu::cache_line_size) state : std::atomic<uint64_t>
    {
    };

    alignas(cpu::cache_line_size) std::atomic<uint64_t> updater_version_{0};

    // There are two arrays, each with the number of expected threads, placed
    // in memory one after another.
    alignas(cpu::cache_line_size) std::unique_ptr<state[]> state_;
    const uint32_t num_threads_;

public:
    struct rd_token
    {
        uint8_t v_;
    };
    enum struct wait_cmd
    {
    };
    static constexpr auto wait    = wait_cmd{1};
    static constexpr auto no_wait = wait_cmd{0};

public:
    class lock_guard
    {
        cfreaks_rcu* inst_;
        uint32_t thread_idx_;
        rd_token rd_tok_;

    public:
        lock_guard(cfreaks_rcu& inst, uint32_t thread_idx) noexcept
        : inst_(&inst)
        , thread_idx_(thread_idx)
        , rd_tok_(inst.read_lock(thread_idx))
        {
        }
        ~lock_guard() noexcept { inst_->read_unlock(thread_idx_, rd_tok_); }

        lock_guard()                             = delete;
        lock_guard(lock_guard&&)                 = delete;
        lock_guard(const lock_guard&)            = delete;
        lock_guard& operator=(lock_guard&&)      = delete;
        lock_guard& operator=(const lock_guard&) = delete;
    };

public:
    cfreaks_rcu()                              = delete;
    cfreaks_rcu(cfreaks_rcu&&)                 = delete;
    cfreaks_rcu(const cfreaks_rcu&)            = delete;
    cfreaks_rcu& operator=(cfreaks_rcu&&)      = delete;
    cfreaks_rcu& operator=(const cfreaks_rcu&) = delete;

    // The constructor takes the number of reader threads which should be a
    // number greater than 0 and less than or equal to 2^31.
    explicit cfreaks_rcu(uint32_t num_threads);
    ~cfreaks_rcu() noexcept;

public:
    // Nested read side locking is allowed and supported
    rd_token read_lock(uint32_t thread_idx) noexcept;
    void read_unlock(uint32_t thread_idx, rd_token) noexcept;

    // This function should be called from the writer/updater thread
    // or from a reader thread but when the latter is outside of a read
    // critical section read_lock/unlock.
    // Returns:
    // - `true` if all reader threads are outside of their corresponding read
    // critical sections . If `wait` is passed as a second argument then
    // the function always returns `true`.
    // - `false` if not all reader threads are outside of their corresponding
    // read critical section
    bool synchronize(wait_cmd) noexcept;
};

} // namespace freak::par
