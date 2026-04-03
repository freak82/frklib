#pragma once

#include <atomic>
#include <cassert>
#include <mutex>
#include <thread>

#include "rw_locked.hpp"

// The whole functionality here is still experimental.
// It's based on
// https://github.com/pramalhe/ConcurrencyFreaks/tree/master/CPP/leftright
// in its classic variant.
namespace freak::par
{
struct read_write_sinit_t
{
}; // Single Initialization
struct read_write_dinit_t
{
}; // Double Initialization
struct read_write_pinit_t
{
}; // Piece-wise Initialization

constexpr read_write_sinit_t read_write_sinit{};
constexpr read_write_dinit_t read_write_dinit{};
constexpr read_write_pinit_t read_write_pinit{};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class read_write
{
    std::atomic<uint16_t> read_idx_{0}; // 0 or 1
    mutable std::atomic<uint32_t> rdrs_cnt_[2] = {{0}, {0}};

    T data_[2];

    // The mutex is put last because it's used to serialize writes and the
    // last are supposed to happen rarely. Thus most of the time the memory
    // location of the mutex should stay cold (unused).
    std::mutex wr_mut_;

public:
    template <typename... Args>
    read_write(read_write_sinit_t,
               Args&&... args) noexcept(noexcept(T(args...)) && noexcept(T()))
    : data_{T(std::forward<Args>(args)...), T()}
    {
    }
    template <typename... Args>
    read_write(read_write_dinit_t,
               const Args&... args) noexcept(noexcept(T(args...)))
    : data_{T(args...), T(args...)}
    {
    }
    template <typename U, typename V>
    read_write(read_write_pinit_t, U&& u, V&& v) noexcept(noexcept(T(u)) &&
                                                          noexcept(T(v)))
    : data_{T(std::forward<U>(u)), T(std::forward<V>(v))}
    {
    }
    ~read_write() noexcept = default;

    read_write(const read_write&)            = delete;
    read_write& operator=(const read_write&) = delete;
    read_write(read_write&&)                 = delete;
    read_write& operator=(read_write&&)      = delete;

    auto read() const noexcept
    {
        const auto ri = read_idx_.load(std::memory_order_acquire);
        // Not sure if I can use memory_order_acq_rel here.
        // We need the code following this increment to not be reordered
        // before it and thus we need acquire semantics.
        // At the same time, we need the store rdrs_cnt_ here to
        // 'synchronizes-with' the load from the rdrs_cnt_ in the write data.
        [[maybe_unused]] const auto prev =
            rdrs_cnt_[ri].fetch_add(1, std::memory_order_seq_cst);
        assert(prev < std::numeric_limits<decltype(prev)>::max());
        const T& d = data_[ri];
        return make_rw_locked(d, [rc = &rdrs_cnt_[ri]] {
            rc->fetch_sub(1, std::memory_order_release);
        });
    }
    auto write() noexcept
    {
        wr_mut_.lock();
        // The write index is the opposite of the read index
        const auto ri = read_idx_.load(std::memory_order_acquire);
        T& d          = data_[ri ^ 0x1];
        return make_rw_locked(d, [this, ri] {
            const uint16_t new_ri = ri ^ 0x1;

            // This wait for the readers from the next index may seem
            // unneeded at first glance but it's actually needed due to the
            // following possible sequence of events.
            // 1. A call to read_data reads 0 from read_idx_.
            // 2. A writer begins and ends flipping the read_idx_ from 0 to 1.
            // The writer will see no previous readers and exit.
            // 3. The reader from point 1 continues its execution and increments
            // the rdrs_cnt_[0] and continue reading the actual data.
            // 4. Another writer begins and ends flipping the read_idx_ from
            // 1 to 0. The writer will see no readers at ri = 1 and return.
            // This way the logic will violate the requirement to wait for the
            // readers from the previous read_idx and thus it'll allow wrongly
            // for the next writer to write to the same data which are currently
            // read.
            wait_no_readers(new_ri);

            // We want this store to 'synchronizes-with' the load from the
            // read_data function. We need at least 'release' semantics for it.
            // However, we also don't want anything of the below code to go
            // before this call. For this we need at least acquire semantics.
            // I use seq_cst because I'm unsure if memory_order_acq_rel will
            // work correctly.
            read_idx_.store(new_ri, std::memory_order_seq_cst);

            // At this point we know that new readers will see only the new_ri.
            // Thus, we need to wait only for the readers from the previous ri.
            wait_no_readers(ri);

            wr_mut_.unlock();
        });
    }

    // Shorter syntax for single operations
    auto operator->() const noexcept { return read(); }
    auto operator->() noexcept { return write(); }

    const read_write& as_const() const noexcept { return *this; }

private:
    void wait_no_readers(uint16_t ri) noexcept
    {
        // This spin/yield loop here can be made more sophisticated/optimal.
        while (rdrs_cnt_[ri].load(std::memory_order_acquire) != 0) {
            std::this_thread::yield();
        }
    }
};

} // namespace freak::par
