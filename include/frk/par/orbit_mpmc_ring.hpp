#pragma once

#include "frk/mem/wrappers.hpp"
#include "frk/par/cpu.hpp"

#include <stdint.h>

#include <atomic>
#include <bit>
#include <memory>
#include <type_traits>
#include <utility>

namespace freak::par
{

// Original source code:
// https://github.com/austin-hill/orbit/blob/main/include/orbit/mpmc_queue.h
template <typename T, typename A = std::allocator<T>>
class orbit_mpmc_ring
{
public:
    using value_type     = T;
    using allocator_type = A;
    using size_type      = uint32_t;

private:
    using atomic_state_type = std::atomic<uint32_t>;
    using atomic_pos_type   = std::atomic<uint32_t>;
    static_assert(atomic_pos_type::is_always_lock_free);
    static_assert(atomic_state_type::is_always_lock_free);
    // Naked pointers are used instead of unique_ptrs with embedded deleters
    // because the later will need the number of elements to be stored in the
    // deleter and thus increasing the size of the whole ring by 8 bytes x86_64
    atomic_state_type* states_;
    value_type* elems_;
    const size_type size_;
    const size_type cycle_shift_;
    // Avoid false sharing between the above variables and these two as well as
    // between these two which are accessed by different threads.
    alignas(cpu::cache_line_size) atomic_pos_type head_{0};
    alignas(cpu::cache_line_size) atomic_pos_type tail_{0};
    // The allocator is used only at construction and destruction time and
    // thus it doesn't matter if it happens to be in the same cache line as
    // the tail.
    [[no_unique_address]] allocator_type alloc_;

public:
    explicit orbit_mpmc_ring(size_type size, const allocator_type& alloc = {})
    : size_(size), cycle_shift_(std::countr_zero(size)), alloc_(alloc)
    {
        // The precondition is that the size MUST BE power of 2
        FRK_ENFORCE(std::has_single_bit(size));
        // If the second throws exception the memory will be correctly
        // cleaned up before the exception is propagated to the caller.
        using traits_type = std::allocator_traits<A>::template rebind_traits<T>;
        auto states = mem::allocate_unique<atomic_state_type[]>(alloc_, size_);
        elems_      = traits_type::allocate(alloc_, size_);
        states_     = states.release();
        for (auto cur = states_, end = states_ + size_; cur != end; ++cur) {
            std::atomic_init(cur, {});
        }
    }
    ~orbit_mpmc_ring() noexcept
    {
        using traits_type = std::allocator_traits<A>::template rebind_traits<T>;
        // Unoptimized destruction of the ring entries
        // The cycle just pops them out while the pop calls the destructor of
        // the entry placed in the ring.
        struct ignore
        {
            void operator=(T&&) const noexcept {}
        };
        std::array<ignore, 16uz> tmp;
        while (try_pop(tmp.data(), tmp.size()) > 0) {}
        traits_type::deallocate(alloc_, elems_, size_);
        // It's important the states information to be destroyed last
        // because the pop functionality uses the states.
        mem::allocator_delete(alloc_, elems_, size_);
    }

    orbit_mpmc_ring()                                  = delete;
    orbit_mpmc_ring(orbit_mpmc_ring&&)                 = delete;
    orbit_mpmc_ring(const orbit_mpmc_ring&)            = delete;
    orbit_mpmc_ring& operator=(orbit_mpmc_ring&&)      = delete;
    orbit_mpmc_ring& operator=(const orbit_mpmc_ring&) = delete;

public:
    template <typename... Args>
        requires(std::is_constructible_v<T, Args...>)
    bool try_push(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>)
    {
        for (;;) {
            const auto tail  = tail_.load(std::memory_order_relaxed);
            const auto cycle = (tail >> cycle_shift_) + 1;
            // Want to set empty slot from last cycle to writing in this cycle
            const auto current = ((cycle - 1) << state_shift) | state::empty;
            const auto writing = (cycle << state_shift) | state::writing;
            const auto written = (cycle << state_shift) | state::written;
            const auto pos     = (tail * step) & (size_ - 1);
            auto& state        = states_[pos];

            if (!state.compare_exchage_weak(current, writing,
                                            std::memory_order_acquire,
                                            std::memory_order_relaxed)) {
                if ((current >> state_shift) == (cycle - 1)) [[unlikely]] {
                    return false; // the ring is full
                }

                continue;
            }

            // Allow the other threads to proceed with the pushing
            tail_.store(tail + 1, std::memory_order_relaxed);

            std::construct_at(std::to_address(&elems_[pos]),
                              std::forward<Args>(args)...);

            state.store(written, std::memory_order_release);
        }
    };

} // namespace freak::par
