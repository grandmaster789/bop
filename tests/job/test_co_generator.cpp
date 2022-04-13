#include <iostream>
#include <memory>

#include "../../src/job/co_generator.h"

#include <catch2/catch.hpp>

namespace testing {
    int test_iota() {
        int sum = 0;

        // derives to int
        for (auto x : bop::job::iota(6))
            sum += x;

        return sum;
    }

    uint32_t test_generator_range_sum() {
        uint32_t sum = 0;

        // explicitly made uint32_t
        for (auto x : bop::job::range<uint32_t>(5, 10))
            sum += x;

        return sum;
    }
}

TEST_CASE("test_generator[iota]") {
    REQUIRE(testing::test_iota() == (0 + 1 + 2 + 3 + 4 + 5)); // == 15
}

TEST_CASE("test_generator[range]") {
    REQUIRE(testing::test_generator_range_sum() == (5 + 6 + 7 + 8 + 9)); // == 35
}