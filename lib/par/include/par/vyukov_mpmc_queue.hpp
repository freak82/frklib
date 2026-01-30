#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <memory>

#include "par/cpu.hpp"

namespace freak::par
{

// TODO: Add allocator support
template <typename T>
class vyukov_mpmc_queue
{
    struct cell
    {
        std::atomic<uint64_t> seq_;

        union u
        {
            T data_;
            u() {}
            ~u() {}
        } u_;
    };

    alignas(cpu::cache_line_size) std::unique_ptr<cell[]> cells_;
    const uint64_t pos_mask_;

    alignas(cpu::cache_line_size) std::atomic<uint64_t> push_pos_{0};

    alignas(cpu::cache_line_size) std::atomic<uint64_t> pop_pos_{0};

public:
    using value_type = T;

public:
    vyukov_mpmc_queue()                                    = delete;
    vyukov_mpmc_queue(vyukov_mpmc_queue&&)                 = delete;
    vyukov_mpmc_queue(const vyukov_mpmc_queue&)            = delete;
    vyukov_mpmc_queue& operator=(vyukov_mpmc_queue&&)      = delete;
    vyukov_mpmc_queue& operator=(const vyukov_mpmc_queue&) = delete;

    // The given size must be greater than 1 and power of 2
    explicit vyukov_mpmc_queue(size_t size)
    : cells_(std::make_unique<cell[]>(size)), pos_mask_(size - 1)
    {
        assert((size >= 2) && ((size & (size - 1)) == 0));

        static_assert(decltype(cell::seq_)::is_always_lock_free);
        static_assert(decltype(push_pos_)::is_always_lock_free);
        static_assert(decltype(pop_pos_)::is_always_lock_free);

        for (size_t i = 0; i < size; ++i) {
            cells_[i].seq_.store(i, std::memory_order_relaxed);
        }
    }

    ~vyukov_mpmc_queue() noexcept
    {
        if constexpr (!std::is_trivially_destructible_v<value_type>) {
            const auto push_pos = push_pos_.load(std::memory_order_relaxed);
            auto pop_pos        = pop_pos_.load(std::memory_order_relaxed);
            for (; pop_pos != push_pos; ++pop_pos) {
                auto* pc = &cells_[pop_pos & pos_mask_];
                assert(pc->seq_.load(std::memory_order_acquire) == pop_pos + 1);
                pc->u_.data_.~value_type();
            }
        }
    }

    // clang-format off
    template <typename... Args>
    bool try_push(Args&&... args) noexcept
        requires(std::is_nothrow_constructible_v<T, Args...>)
    // clang-format on
    {
        cell* pc     = nullptr;
        uint64_t pos = push_pos_.load(std::memory_order_relaxed);

        for (;;) {
            pc = &cells_[pos & pos_mask_];

            const auto seq = pc->seq_.load(std::memory_order_acquire);
            const auto dif =
                static_cast<int64_t>(seq) - static_cast<int64_t>(pos);
            if (dif == 0) {
                if (push_pos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (dif > 0) {
                pos = push_pos_.load(std::memory_order_relaxed);
            } else {
                // The queue is full
                return false;
            }
        }

        new (&pc->u_.data_) value_type(std::forward<Args>(args)...);
        pc->seq_.store(pos + 1, std::memory_order_release);

        return true;
    }

    // clang-format off
    template <typename U>
    bool try_pop(U& to) noexcept
        requires(std::is_nothrow_assignable_v<U&, T&&>)
    // clang-format on
    {
        return try_pop_fn([&to](T&& v) noexcept { to = std::move(v); });
    }

    // clang-format off
    template <typename Fn>
    bool try_pop_fn(Fn&& fn) noexcept
        requires(std::is_nothrow_invocable_v<Fn, value_type&&>)
    // clang-format on
    {
        cell* pc     = nullptr;
        uint64_t pos = pop_pos_.load(std::memory_order_relaxed);

        for (;;) {
            pc = &cells_[pos & pos_mask_];

            const auto seq = pc->seq_.load(std::memory_order_acquire);
            const auto dif =
                static_cast<int64_t>(seq) - static_cast<int64_t>(pos + 1);
            if (dif == 0) {
                if (pop_pos_.compare_exchange_weak(pos, pos + 1,
                                                   std::memory_order_relaxed)) {
                    break;
                }
            } else if (dif > 0) {
                pos = pop_pos_.load(std::memory_order_relaxed);
            } else {
                // The queue is empty
                return false;
            }
        }

        std::forward<Fn>(fn)(std::move(pc->u_.data_));
        pc->u_.data_.~value_type();
        pc->seq_.store(pos + 1 + pos_mask_, std::memory_order_release);

        return true;
    }
};

} // namespace freak::par
