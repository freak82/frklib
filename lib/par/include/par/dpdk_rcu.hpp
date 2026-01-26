#pragma once

#include <assert.h>
#include <stdint.h>

#include <atomic>
#include <memory>

namespace freak::par
{
// This implements Quiescent State Based Reclamation (QSBR).
//
// Quiescent State (QS) is any point in the thread execution
// where the thread does not hold a reference to a data structure
// in shared memory. While using lock-free data structures, the writer
// can safely free memory once all the reader threads have entered
// quiescent state.
//
// This functionality provides the ability for the readers to report quiescent
// state and for the writers to identify when all the readers have
// entered quiescent state.
//
// TODO: Add allocator support
class dpdk_rcu
{
    static constexpr auto cache_line_size = 64uz;

    // Quiescent state counter
    // Needs to be aligned to avoid the false sharing between the threads
    struct alignas(cache_line_size) qsbr_cnt : std::atomic<uint64_t>
    {
    };

    alignas(cache_line_size) std::atomic<uint64_t> current_token_{1};
    std::atomic<uint64_t> acked_token_{0};

    // There is one quiescent state counter for every reader thread.
    alignas(cache_line_size) std::unique_ptr<qsbr_cnt[]> qsbr_cnt_;
    const uint32_t num_threads_;

public:
    struct qs_token
    {
        uint64_t v_;
    };
    enum struct qs_wait_cmd
    {
    };
    static constexpr auto qs_wait    = qs_wait_cmd{1};
    static constexpr auto qs_no_wait = qs_wait_cmd{0};

public:
    dpdk_rcu()                           = delete;
    dpdk_rcu(dpdk_rcu&&)                 = delete;
    dpdk_rcu(const dpdk_rcu&)            = delete;
    dpdk_rcu& operator=(dpdk_rcu&&)      = delete;
    dpdk_rcu& operator=(const dpdk_rcu&) = delete;

    // The constructor takes the number of reader threads which should be a
    // number greater than 0.
    explicit dpdk_rcu(uint32_t num_threads);
    ~dpdk_rcu() noexcept;

    // This function should be called from the reader threads when the
    // corresponding thread does not hold a reference to a shared data structure
    // The given `thread_idx` should be in the range [0 - `num_threads`).
    void reader_quiescent_state(uint32_t thread_idx) noexcept;

public:
    // This function should be called from the writer/updater thread
    // or from a reader thread but after calling `reader_quiescent_state`.
    void synchronize() noexcept
    {
        check_quiescent_state(start_update(), qs_wait);
    }

    // This function should be called from the writer/updater thread
    // or from a reader thread at any point.
    [[nodiscard]] qs_token start_update() noexcept
    {
        // This one synchronizes with the load in the `reader_quiescent_state`.
        return {current_token_.fetch_add(1, std::memory_order_release) + 1};
    }
    // This function should be called from the writer/updater thread
    // or from a reader thread but after calling `reader_quiescent_state`.
    // Returns:
    // - `true` if all reader threads have passed the quiescent state
    // corresponding to the provided token. If `qs_wait` is passed as a second
    // argument then the function always returns `true`.
    // - `false` if not all reader threads have passed the quiescent state
    // corresponding to the provided token
    bool check_quiescent_state(qs_token, qs_wait_cmd) const noexcept;
};

} // namespace freak::par
