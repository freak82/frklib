#include "par/dpdk_rcu.hpp"

#include <assert.h>

namespace freak::par
{

dpdk_rcu::dpdk_rcu(uint32_t num_threads)
: qsbr_cnt_(std::make_unique<qsbr_cnt[]>(num_threads))
, num_threads_(num_threads)
{
    assert(num_threads > 0);

    static_assert(qsbr_cnt::is_always_lock_free);
    static_assert(decltype(current_token_)::is_always_lock_free);
    static_assert(decltype(acked_token_)::is_always_lock_free);

    // Since C++ 20 the default constructor of the std::atomic does
    // value initialization and thus we don't need to explicitly set the
    // thread counters to 0.
}

dpdk_rcu::~dpdk_rcu() noexcept = default;

void dpdk_rcu::reader_quiescent_state(uint32_t thread_idx) noexcept
{
    assert(thread_idx < num_threads_);
    // This one synchronizes with the store in the `start_update`.
    // It also prevents the later loads being moved before it.
    const auto tok = current_token_.load(std::memory_order_acquire);
    // Prior loads to the shared data structure should not be moved beyond
    // this store.
    if (auto& cnt = qsbr_cnt_[thread_idx];
        tok != cnt.load(std::memory_order_relaxed)) {
        cnt.store(tok, std::memory_order_release);
    }
}

bool dpdk_rcu::check_quiescent_state(qs_token tok, qs_wait_cmd wait) noexcept
{
    static constexpr uint64_t max_acked_token = ~0ull;

    // This is just an optimization for the case of multiple writers and thus
    // we don't need to be precise and don't need memory ordering/barriers.
    if (tok.v_ <= acked_token_.load(std::memory_order_relaxed)) return true;

    uint64_t acked_token = max_acked_token;

    for (const auto& cnt : std::span(qsbr_cnt_.get(), num_threads_)) {
        for (;;) {
            // The functionality doesn't check for wrap-around condition
            // because 2^64 updates are not expected to happen in practice for
            // any application that I can imagine.
            if (auto c = cnt.load(std::memory_order_acquire); c >= tok.v_) {
                // Set the acknowledged token to the smallest of all readers.
                if (acked_token > c) acked_token = c;
                break;
            }
            if (wait == qs_no_wait) return false;
            cpu::pause();
        }
    }

    if (acked_token != max_acked_token)
        acked_token_.store(acked_token, std::memory_order_relaxed);

    return true;
}

} // namespace freak::par
