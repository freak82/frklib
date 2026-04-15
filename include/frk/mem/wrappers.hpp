#pragma once

#include <memory>
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
        traits_type::construct(a, std::to_address(p),
                               std::forward<Args>(args)...);
        return p;
    } catch (...) {
        traits_type::deallocate(a, p, 1);
        throw;
    }
}

template <typename A, typename P>
void allocator_delete(A& alloc, P p)
{
    using elem_type = std::pointer_traits<P>::element_type;
    using traits_type =
        std::allocator_traits<A>::template rebind_traits<elem_type>;

    traits_type::destroy(alloc, std::to_address(p));
    traits_type::deallocate(alloc, p, 1);
}

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

template <class T, class A, class... Args>
auto allocate_unique(A& alloc, Args&&... args)
{
    using alloc_type = std::allocator_traits<A>::template rebind_alloc<T>;
    return std::unique_ptr<T, allocation_deleter<alloc_type>>(
        allocator_new<T>(alloc, std::forward<Args>(args)...), alloc);
}

} // namespace frk::mem
