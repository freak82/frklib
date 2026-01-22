#pragma once

#include <assert.h>
#include <stdint.h>

#include <atomic>
#include <memory>

namespace freak::par
{

template <typename T>
class vyukov_mpmc_queue
{
    // The position/sequence need to be limited to 63 bits due to the need of
    // one bit for a flag if the corresponding `data_` has been constructed
    // or not.
    static constexpr uint64_t seq_mask = (1ull << 63) - 1;
    struct cell
    {
        struct flag_seq
        {
            uint64_t flag_ : 1;
            uint64_t seq_  : 63;
        };
        std::atomic<flag_seq> flag_seq_;

        union u
        {
            T data_;
            u() {}
            ~u() {}
        } u_;
    };

    static constexpr size_t cache_line_size = 64;

    alignas(cache_line_size) std::unique_ptr<cell[]> cells_;
    const uint64_t pos_mask_;

    alignas(cache_line_size) std::atomic<uint64_t> push_pos_{0};

    alignas(cache_line_size) std::atomic<uint64_t> pop_pos_{0};

public:
    using value_type = T;

public:
    vyukov_mpmc_queue()                                    = delete;
    vyukov_mpmc_queue(vyukov_mpmc_queue&&)                 = delete;
    vyukov_mpmc_queue(const vyukov_mpmc_queue&)            = delete;
    vyukov_mpmc_queue& operator=(vyukov_mpmc_queue&&)      = delete;
    vyukov_mpmc_queue& operator=(const vyukov_mpmc_queue&) = delete;

    // The given size must be greater than 1 and smaller than 2^63 and
    // power of 2
    explicit vyukov_mpmc_queue(size_t size)
    : cells_(std::make_unique<cell[]>(size)), pos_mask_(size - 1)
    {
        assert((size >= 2) && (size <= seq_mask) && ((size & (size - 1)) == 0));

        static_assert(decltype(cell::flag_seq_)::is_always_lock_free);
        static_assert(decltype(push_pos_)::is_always_lock_free);
        static_assert(decltype(pop_pos_)::is_always_lock_free);

        for (size_t i = 0; i < size; ++i) {
            cells_[i].flag_seq_.store(
                {
                    .flag_ = false,
                    .seq_  = i & seq_mask,
                },
                std::memory_order_relaxed);
        }
    }

    ~vyukov_mpmc_queue() noexcept
    {
        if constexpr (!std::is_trivially_destructible_v<value_type>) {
            const auto size = pos_mask_ + 1;
            for (auto& c : std::span(cells_.get(), size)) {
                const auto fc = c.flag_seq_.load(std::memory_order_relaxed);
                if (fc.flag_ == true) [[likely]] { c.u_.data_.~value_type(); }
            }
        }
    }

    // clang-format off
    template <typename... Args>
    bool try_push(Args&&... args) 
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
        requires(std::is_constructible_v<T, Args...>)
    // clang-format on
    {
        // cell* pc = nullptr;
        // const auto pos = push_pos_.load(std::memory_order_relaxed);
        // for (;;) {

        //}

        return true;
    }
};

/*
template <typename T>
class mpmc_bounded_queue

{
public:
    mpmc_bounded_queue(size_t buffer_size)

    : buffer_(new cell_t[buffer_size])

    , buffer_mask_(buffer_size - 1)

    {
        assert((buffer_size >= 2) &&

               ((buffer_size & (buffer_size - 1)) == 0));

        for (size_t i = 0; i != buffer_size; i += 1)

            buffer_[i].sequence_.store(i, std::memory_order_relaxed);

        enqueue_pos_.store(0, std::memory_order_relaxed);

        dequeue_pos_.store(0, std::memory_order_relaxed);
    }

    ~mpmc_bounded_queue() { delete[] buffer_; }

    bool enqueue(T const& data)

    {
        cell_t* cell;

        size_t pos = enqueue_pos_.load(std::memory_order_relaxed);

        for (;;)

        {
            cell = &buffer_[pos & buffer_mask_];

            size_t seq =

                cell->sequence_.load(std::memory_order_acquire);

            intptr_t dif = (intptr_t)seq - (intptr_t)pos;

            if (dif == 0)

            {
                if (enqueue_pos_.compare_exchange_weak

                    (pos, pos + 1, std::memory_order_relaxed))

                    break;

            }

            else if (dif < 0)

                return false;

            else

                pos = enqueue_pos_.load(std::memory_order_relaxed);
        }

        cell->data_ = data;

        cell->sequence_.store(pos + 1, std::memory_order_release);

        return true;
    }

    bool dequeue(T& data)

    {
        cell_t* cell;

        size_t pos = dequeue_pos_.load(std::memory_order_relaxed);

        for (;;)

        {
            cell = &buffer_[pos & buffer_mask_];

            size_t seq =

                cell->sequence_.load(std::memory_order_acquire);

            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);

            if (dif == 0)

            {
                if (dequeue_pos_.compare_exchange_weak

                    (pos, pos + 1, std::memory_order_relaxed))

                    break;

            }

            else if (dif < 0)

                return false;

            else

                pos = dequeue_pos_.load(std::memory_order_relaxed);
        }

        data = cell->data_;

        cell->sequence_.store

            (pos + buffer_mask_ + 1, std::memory_order_release);

        return true;
    }

private:
    struct cell_t

    {
        std::atomic<size_t> sequence_;

        T data_;
    };

    static size_t const cacheline_size = 64;

    typedef char cacheline_pad_t[cacheline_size];

    cacheline_pad_t pad0_;

    cell_t* const buffer_;

    size_t const buffer_mask_;

    cacheline_pad_t pad1_;

    std::atomic<size_t> enqueue_pos_;

    cacheline_pad_t pad2_;

    std::atomic<size_t> dequeue_pos_;

    cacheline_pad_t pad3_;

    mpmc_bounded_queue(mpmc_bounded_queue const&);

    void operator=(mpmc_bounded_queue const&);
};
*/

} // namespace freak::par
