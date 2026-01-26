#include "par/dpdk_rcu.hpp"

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

bool dpdk_rcu::check_quiescent_state(qs_token, qs_wait_cmd) const noexcept
{
    // TODO
    return false;
}

} // namespace freak::par
