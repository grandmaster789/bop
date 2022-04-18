#pragma once

namespace bop::util {
	template <typename...Ts>
	struct Overloaded : Ts... {
		using Ts::operator()...;
	};

	/*
	* usage:
	* 
	* std::variant<int, long, double> some_variant;
	* 
	* std::visit(util::Overloaded {
	*	[](auto   arg) { std::cout << "Catch-all: " << arg << "\n"; },
	*	[](double arg) { std::cout << "Double: "    << arg << "\n"; },
	*   [](int    arg) { std::cout << "Int: "       << arg << "\n"; }
	* }, some_variant);
	*/
}