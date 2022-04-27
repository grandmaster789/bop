#include "../../src/util/function.h"

#include <catch2/catch.hpp>

namespace testing {
	int ping() { return 42; }
}

TEST_CASE("test_store_function_ptr") {
	util::Function<int()> fn = testing::ping;

	REQUIRE(fn() == 42);
}

TEST_CASE("test_store_lambda") {
	util::Function<int(int)> fn = [](int x) { return x * x; };

	REQUIRE(fn(5) == 25);

	// rebind to another lambda
	fn = [](int x) { return x * 2; };

	REQUIRE(fn(6) == 12);
}

TEST_CASE("test_empty_function") {
	util::Function<int()> fn;

	REQUIRE(!fn);
	REQUIRE_THROWS(fn());
}
