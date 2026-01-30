#include "par/cfreaks_rcu.hpp"

#include <assert.h>

namespace freak::par
{

cfreaks_rcu::cfreaks_rcu(uint32_t num_threads)
: state_(std::make_unique<state[]>(2 * num_threads)), num_threads_(num_threads)
{
    assert(num_threads > 0);
    assert(num_threads <= (UINT_MAX >> 1));

    static_assert(state::is_always_lock_free);
    static_assert(decltype(updater_version_)::is_always_lock_free);

    // Since C++ 20 the default constructor of the std::atomic does
    // value initialization and thus we don't need to explicitly set the
    // thread states to false.
}

cfreaks_rcu::~cfreaks_rcu() noexcept = default;

cfreaks_rcu::rd_token cfreaks_rcu::read_lock(uint32_t thread_idx) noexcept
{
    assert(thread_idx < num_threads_);

    // This load synchronizes with the store to the same variable in the
    // `synchronize` function.
    const auto upd_pos = updater_version_.load(std::memory_order_acquire) & 0x1;
    const auto upd_idx = (upd_pos * num_threads_) + thread_idx;
    // Even though this function is not put in the header i.e. the compiler
    // will treat it as non-opaque i.e. memory barrier, a build with LTO enabled
    // can change this and thus we need to be prepared.
    // We need a release store here which synchronizes with the acquire load
    // in the `check_quiescent_state`.
    // However, we also need acquire barrier to prevent code being moved above
    // the read side critical section. A separate atomic_thread_fence can't be
    // used here because it won't synchronizes with anything unless we use an
    // artificial separate barrier in the `read_unlock`.
    // Another option is to just do an acquire load operation after the store
    // here and throw away its result but that just seems to me as "heavy" as
    // single exchange operation here.
    state_[upd_idx].fetch_add(1, std::memory_order_acq_rel);
    return {.v_ = static_cast<uint8_t>(upd_pos)};
}

void cfreaks_rcu::read_unlock(uint32_t thread_idx, rd_token rdt) noexcept
{
    assert(rdt.v_ < 2u);
    assert(thread_idx < num_threads_);
    const auto upd_idx = (rdt.v_ * num_threads_) + thread_idx;
    [[maybe_unused]] const auto st =
        state_[upd_idx].fetch_sub(1, std::memory_order_release);
    // An assert here the `read_lock`/`read_unlock` functions are not paired
    // correctly by the user of the RCU.
    assert(st >= 1ul);
}

bool cfreaks_rcu::synchronize(wait_cmd cmd) noexcept
{
    // The functionality doesn't check for wrap-around condition
    // because 2^64 updates are not expected to happen in practice for
    // any application that I can imagine.
    const auto curr_ver = updater_version_.load(std::memory_order_acquire);
    const auto next_ver = (curr_ver + 1);
    auto readers_done   = [this](uint8_t pos) {
        for (auto& st : std::span(&state_[pos * num_threads_], num_threads_)) {
            if (st.load(std::memory_order_acquire) != 0) return false;
        }
        return true;
    };
    // Wait until all older readers have moved to the currently active updater
    // version.
    // Note that another updater may have succeeded increasing the update
    // version or even finishing the whole `synchronize` procedure.
    // In the latter case we consider our call to `synchronize` to be successful
    // in the waiting and return true.
    while (!readers_done(next_ver & 0x1)) {
        if (const auto cv = updater_version_.load(std::memory_order_acquire);
            cv == next_ver) {
            break;
        } else if (cv != curr_ver) {
            return true;
        }
        if (cmd == no_wait) return false;
        cpu::pause();
    }
    // Another thread may have already updated the version to a bigger number
    // and thus we need the compare and swap.
    // The store here synchronizes with all of the acquire loads of the same
    // variable in this and the other functions.
    auto tmp = curr_ver;
    updater_version_.compare_exchange_strong(tmp, next_ver,
                                             std::memory_order_acq_rel);
    // Wait until all readers move to the new updater version.
    // Note that from this point on all loads from the updater version will see
    // the next version or versions after it. So, if the `updater_version_`
    // is not equal to the `next_ver` then some other thread has waited the
    // reader threads and updated the version i.e. we are done.
    while (!readers_done(curr_ver & 0x1)) {
        if (const auto cv = updater_version_.load(std::memory_order_acquire);
            cv != next_ver) {
            break;
        }
        // We can't skip the waiting in this case, no matter of the `wait_cmd`
        // because we've already updated the version and would be wrong to
        // return early false here if the cmd is `no_wait`.
        cpu::pause();
    }
    return true;
}

} // namespace freak::par
