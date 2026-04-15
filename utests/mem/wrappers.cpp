#include "frk/mem/wrappers.hpp"

#include <doctest/doctest.h>

#include <stddef.h>

#include <exception>
#include <memory_resource>
#include <new>

namespace
{

class counting_memory_resource : public std::pmr::memory_resource
{
public:
    size_t allocation_count   = 0;
    size_t deallocation_count = 0;
    size_t allocated_bytes    = 0;
    size_t deallocated_bytes  = 0;

private:
    void* do_allocate(size_t bytes, size_t alignment) override
    {
        allocation_count += 1;
        allocated_bytes += bytes;
        return ::operator new(bytes, std::align_val_t(alignment));
    }

    void do_deallocate(void* p, size_t bytes, size_t alignment) override
    {
        deallocation_count += 1;
        deallocated_bytes += bytes;
        ::operator delete(p, bytes, std::align_val_t(alignment));
    }

    bool
    do_is_equal(const std::pmr::memory_resource& rhs) const noexcept override
    {
        return this == &rhs;
    }
};

struct counted_value
{
    static inline size_t construction_count = 0;
    static inline size_t destruction_count  = 0;

    int value = 0;

    explicit counted_value(int v) : value(v) { ++construction_count; }
    ~counted_value() { ++destruction_count; }

    static void reset() noexcept
    {
        construction_count = 0;
        destruction_count  = 0;
    }
};

struct thrown_exception : std::exception
{
};

struct throwing_value
{
    explicit throwing_value(int) { throw thrown_exception(); }
};

} // namespace
////////////////////////////////////////////////////////////////////////////////
DOCTEST_TEST_SUITE_BEGIN("mem/wrappers");

DOCTEST_TEST_CASE("mem::allocator_new_delete")
{
    // GIVEN
    counted_value::reset();
    counting_memory_resource resource;
    std::pmr::polymorphic_allocator<counted_value> alloc(&resource);

    // WHEN
    auto ptr = frk::mem::allocator_new<counted_value>(alloc, 42);

    // THEN
    DOCTEST_CHECK_EQ(ptr->value, 42);
    DOCTEST_CHECK_EQ(resource.allocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.deallocation_count, 0uz);
    DOCTEST_CHECK_EQ(resource.allocated_bytes, sizeof(counted_value));
    DOCTEST_CHECK_EQ(resource.deallocated_bytes, 0uz);
    DOCTEST_CHECK_EQ(counted_value::construction_count, 1uz);
    DOCTEST_CHECK_EQ(counted_value::destruction_count, 0uz);

    // WHEN
    frk::mem::allocator_delete(alloc, ptr);

    // THEN
    DOCTEST_CHECK_EQ(resource.allocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.deallocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.allocated_bytes, sizeof(counted_value));
    DOCTEST_CHECK_EQ(resource.deallocated_bytes, sizeof(counted_value));
    DOCTEST_CHECK_EQ(counted_value::construction_count, 1uz);
    DOCTEST_CHECK_EQ(counted_value::destruction_count, 1uz);
}

DOCTEST_TEST_CASE("mem::allocator_new_throw")
{
    // GIVEN
    counting_memory_resource resource;
    std::pmr::polymorphic_allocator<throwing_value> alloc(&resource);

    // WHEN
    DOCTEST_CHECK_THROWS_AS(frk::mem::allocator_new<throwing_value>(alloc, 7),
                            thrown_exception);

    // THEN
    DOCTEST_CHECK_EQ(resource.allocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.deallocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.allocated_bytes, sizeof(throwing_value));
    DOCTEST_CHECK_EQ(resource.deallocated_bytes, sizeof(throwing_value));
}

DOCTEST_TEST_CASE("mem::allocate_unique")
{
    // GIVEN
    counted_value::reset();
    counting_memory_resource resource;
    std::pmr::polymorphic_allocator<counted_value> alloc(&resource);

    // WHEN
    auto ptr = frk::mem::allocate_unique<counted_value>(alloc, 24);

    // THEN
    DOCTEST_REQUIRE(ptr);
    DOCTEST_CHECK_EQ(ptr->value, 24);
    DOCTEST_CHECK_EQ(resource.allocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.deallocation_count, 0uz);
    DOCTEST_CHECK_EQ(resource.allocated_bytes, sizeof(counted_value));
    DOCTEST_CHECK_EQ(resource.deallocated_bytes, 0uz);
    DOCTEST_CHECK_EQ(counted_value::construction_count, 1uz);
    DOCTEST_CHECK_EQ(counted_value::destruction_count, 0uz);

    // WHEN
    ptr.reset();

    // THEN
    DOCTEST_CHECK_EQ(resource.allocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.deallocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.allocated_bytes, sizeof(counted_value));
    DOCTEST_CHECK_EQ(resource.deallocated_bytes, sizeof(counted_value));
    DOCTEST_CHECK_EQ(counted_value::construction_count, 1uz);
    DOCTEST_CHECK_EQ(counted_value::destruction_count, 1uz);
}

DOCTEST_TEST_CASE("mem::allocate_unique_throw")
{
    // GIVEN
    counting_memory_resource resource;
    std::pmr::polymorphic_allocator<throwing_value> alloc(&resource);

    // WHEN
    DOCTEST_CHECK_THROWS_AS(frk::mem::allocate_unique<throwing_value>(alloc, 9),
                            thrown_exception);

    // THEN
    DOCTEST_CHECK_EQ(resource.allocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.deallocation_count, 1uz);
    DOCTEST_CHECK_EQ(resource.allocated_bytes, sizeof(throwing_value));
    DOCTEST_CHECK_EQ(resource.deallocated_bytes, sizeof(throwing_value));
}

DOCTEST_TEST_SUITE_END();
