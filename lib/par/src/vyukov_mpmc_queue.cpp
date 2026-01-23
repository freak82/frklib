// This file is used only to verify that the corresponding .hpp file can be
// built in isolation i.e. it includes all of the needed headers.
#include "par/vyukov_mpmc_queue.hpp"

[[maybe_unused]]
static void verify_compilation()
{
    freak::par::vyukov_mpmc_queue<int> q(2);
    q.try_push(42);
    q.try_push(84);
    int i = 0;
    q.try_pop(i);
    q.try_pop_fn([&i](int ii) noexcept { i = ii; });
}
