// This file is used only to verify that the corresponding .hpp file can be
// built in isolation i.e. it includes all of the needed headers.
#include "par/vyukov_mpmc_queue.hpp"

[[maybe_unused]]
static void verify_compilation()
{
    [[maybe_unused]] freak::par::vyukov_mpmc_queue<int> q(2);
}
