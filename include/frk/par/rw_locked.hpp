#pragma once

#include <utility>

namespace freak::par
{

template <typename T, typename UnlockFn>
class rw_locked : private UnlockFn
{
    T* data_;

public:
    rw_locked(T& d, const UnlockFn& fn) noexcept : UnlockFn{fn}, data_{&d} {}
    rw_locked(rw_locked&& rhs) noexcept
    : UnlockFn{std::move(static_cast<UnlockFn&&>(rhs))}
    , data_{std::exchange(rhs.data_, nullptr)}
    {
    }
    ~rw_locked() noexcept
    {
        if (data_) this->operator()();
    }

    rw_locked(const rw_locked&)            = delete;
    rw_locked& operator=(const rw_locked&) = delete;
    rw_locked& operator=(rw_locked&&)      = delete;

    T* operator->() const noexcept { return data_; }
    T& operator*() const noexcept { return *data_; }

    explicit operator bool() const noexcept { return !!data_; }
};

template <typename T, typename UnlockFn>
rw_locked<T, UnlockFn> make_rw_locked(T& d, const UnlockFn& fn) noexcept
{
    return {d, fn};
}

} // namespace freak::par
