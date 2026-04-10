// This file is used only to verify that the corresponding .hpp file can be
// built in isolation i.e. it includes all of the needed headers.
#include "par/read_write.hpp"

[[maybe_unused]]
static void verify_compilation()
{
    struct test
    {
        int i_ = 0;

        test() = default;
    };

    freak::par::read_write<test> rw(freak::par::read_write_dinit);
    rw.write()->i_                = 84;
    [[maybe_unused]] const auto v = rw.read()->i_;
}
