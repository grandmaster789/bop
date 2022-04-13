#include <iostream>
#include <memory>

#include "../../src/job/job_system.h"
#include "../../src/job/co_generator.h"

#include <catch2/catch.hpp>

namespace testing {
    // because of the statics and one-time initialization, we're kind of limited
    // in setup/teardown of the scheduler. Hence, we only have a single file
    // with a single scheduler configuration that is tested.
    //
    // We could possibly adress that, but it would mean getting rid of all static
    // variables in the scheduler, and re-expressing shared queue access between 
    // worker threads by going via some kind of parent object. 
    // 
    // It's probably going to be trickier than the current setup.

    std::atomic<uint32_t> g_Total;

    uint32_t test_single_job() {
        g_Total = 1111;

        auto& job = bop::schedule([&] { g_Total = 2222; });
        job.wait();
        
        return g_Total;
    }

    uint32_t test_generator_range_sum() {
        uint32_t sum = 0;

        for (auto x : bop::job::generator_range<uint32_t>(5, 10))
            sum += x;

        return sum;
    }
}

TEST_CASE("test_scheduler", "[single_job]") {
    REQUIRE(testing::test_single_job() == 2222);
}

TEST_CASE("test_generator", "[integral_range]") {
    REQUIRE(testing::test_generator_range_sum() == (5 + 6 + 7 + 8 + 9)); // == 35
}