#include "frk/algo/algorithm.hpp"

#include <list>

#include <doctest/doctest.h>

////////////////////////////////////////////////////////////////////////////////

DOCTEST_TEST_SUITE_BEGIN("algo/algorithm");

DOCTEST_TEST_CASE("algo::all_of-true")
{
    // GIVEN
    const int rng[] = {1, 1, 1, 1};

    // WHEN
    const bool res = frk::algo::all_of(rng, 1);

    // THEN
    DOCTEST_CHECK_EQ(res, true);
}

DOCTEST_TEST_CASE("algo::all_of-false")
{
    // GIVEN
    const int rng[] = {1, 1, 1, 2};

    // WHEN
    const bool res = frk::algo::all_of(rng, 1);

    // THEN
    DOCTEST_CHECK_EQ(res, false);
}

DOCTEST_TEST_CASE("algo::all_of-bidir-rng")
{
    // GIVEN
    const std::list<int> rng = {1, 1, 1, 1};

    // WHEN
    const bool res = frk::algo::all_of(rng, 1);

    // THEN
    DOCTEST_CHECK_EQ(res, true);
}

DOCTEST_TEST_CASE("algo::all_of-constexpr")
{
    // GIVEN
    constexpr int rng[] = {1, 1, 1, 1};

    // WHEN
    constexpr bool res = frk::algo::all_of(rng, 1);

    // THEN
    static_assert(res == true);
    DOCTEST_CHECK_EQ(res, true);
}

DOCTEST_TEST_SUITE_END();
