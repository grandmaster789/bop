#include <iostream>
#include <memory>

#include <catch2/catch.hpp>

unsigned int factorial(unsigned int number) {
    if (number <= 1)
        return number;
    else
        return factorial(number - 1) * number;
}

TEST_CASE("testing", "[factorial]") {
    REQUIRE(factorial(1) == 1);
    REQUIRE(factorial(6) == 720);
}