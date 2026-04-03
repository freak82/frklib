// This file is used only to verify that the corresponding .hpp file can be
// built in isolation i.e. it includes all of the needed headers.
#include "par/synchronized.hpp"
#include <shared_mutex>
#include <utility>

[[maybe_unused]]
static void verify_compilation()
{
    struct test
    {
        int i_ = 0;

        test() = default;
    };
    freak::par::synchronized<test, std::shared_mutex> synced;
    synced->i_                    = 84;
    [[maybe_unused]] const auto v = std::as_const(synced)->i_;
}
