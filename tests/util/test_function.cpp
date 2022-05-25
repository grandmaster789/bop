#include "../../src/util/function.h"

#include <catch2/catch.hpp>

namespace testing {
	int ping() { return 42; }
}

TEST_CASE("test_store_function_ptr") {
	bop::util::Function<int()> fn = testing::ping;

	REQUIRE(fn() == 42);
}

TEST_CASE("test_store_lambda") {
	bop::util::Function<int(int)> fn = [](int x) { return x * x; };

	REQUIRE(fn(5) == 25);

	fn = [](int x) { return x * 2; }; // rebind to another lambda

	REQUIRE(fn(6) == 12);
}

TEST_CASE("test_empty_function") {
	bop::util::Function<int()> fn;

	REQUIRE(!fn);
	REQUIRE_THROWS(fn());
}
