#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace frk::mem
{

// Functions "stolen" from https://wg21.link/p0211
template <typename T, typename A, typename... Args>
auto allocator_new(A& alloc, Args&&... args)
{
    using alloc_type  = std::allocator_traits<A>::template rebind_alloc<T>;
    using traits_type = std::allocator_traits<A>::template rebind_traits<T>;

    auto a = alloc_type(alloc);
    auto p = traits_type::allocate(a, 1);

    try {
        return std::construct_at(std::to_address(p),
                                 std::forward<Args>(args)...);
    } catch (...) {
        traits_type::deallocate(a, p, 1);
        throw;
    }
}

// TODO: Add unit tests for the array allocation functionality
template <typename T, typename A>
    requires std::is_unbounded_array_v<T>
auto allocator_new(A& alloc, size_t cnt)
{
    using E           = std::remove_extent_t<T>;
    using alloc_type  = std::allocator_traits<A>::template rebind_alloc<E>;
    using traits_type = std::allocator_traits<A>::template rebind_traits<E>;

    auto a = alloc_type(alloc);
    auto p = traits_type::allocate(a, cnt);

    try {
        std::uninitialized_default_construct_n(std::to_address(p), cnt);
        return p;
    } catch (...) {
        traits_type::deallocate(a, p, cnt);
        throw;
    }
}

template <typename T, typename A>
    requires std::is_bounded_array_v<T>
void allocator_new(A& alloc) = delete;

template <typename A, typename P>
void allocator_delete(A& alloc, P p)
{
    using elem_type = std::pointer_traits<P>::element_type;
    using traits_type =
        std::allocator_traits<A>::template rebind_traits<elem_type>;

    std::destroy_at(std::to_address(p));
    traits_type::deallocate(alloc, p, 1);
}

template <typename A, typename P>
void allocator_delete(A& alloc, P p, size_t n)
{
    using elem_type = std::pointer_traits<P>::element_type;
    using traits_type =
        std::allocator_traits<A>::template rebind_traits<elem_type>;

    std::destroy_n(std::to_address(p), n);
    traits_type::deallocate(alloc, p, n);
}

////////////////////////////////////////////////////////////////////////////////

template <class A>
class allocation_deleter
{
    [[no_unique_address]] A alloc_;

public:
    using allocator_type = A;
    using pointer        = std::allocator_traits<A>::pointer;

public:
    allocation_deleter() noexcept = default;
    // Intentionally implicit
    allocation_deleter(const A& a) noexcept : alloc_(a) {}

    void operator()(pointer p) { allocator_delete(alloc_, p); }
};

template <class A>
class array_allocation_deleter
{
    using traits_type = std::allocator_traits<A>;

    traits_type::size_type cnt_ = 0;
    [[no_unique_address]] A alloc_;

public:
    using allocator_type = A;
    using size_type      = traits_type::size_type;
    using pointer        = traits_type::pointer;

public:
    array_allocation_deleter() noexcept = default;
    array_allocation_deleter(const A& a, size_type c) noexcept
    : cnt_(c), alloc_(a)
    {
    }

    void operator()(pointer p) { allocator_delete(alloc_, p, cnt_); }
};

////////////////////////////////////////////////////////////////////////////////

template <class T, class A, class... Args>
auto allocate_unique(A& alloc, Args&&... args)
{
    using alloc_type = std::allocator_traits<A>::template rebind_alloc<T>;
    return std::unique_ptr<T, allocation_deleter<alloc_type>>(
        allocator_new<T>(alloc, std::forward<Args>(args)...), alloc);
}

template <class T, class A>
    requires std::is_unbounded_array_v<T>
auto allocate_unique(A& alloc, size_t cnt)
{
    using E            = std::remove_extent_t<T>;
    using alloc_type   = std::allocator_traits<A>::template rebind_alloc<E>;
    using deleter_type = array_allocation_deleter<alloc_type>;
    return std::unique_ptr<T, deleter_type>(allocator_new<T>(alloc, cnt),
                                            deleter_type(alloc, cnt));
}

template <class T, class A, class... Args>
    requires std::is_bounded_array_v<T>
void allocate_unique(A& alloc, Args&&... args) = delete;

} // namespace frk::mem
