#pragma once

#include <functional>

namespace bop::util {
	// from C++Weekly 318
	// very neat, but it does make a copy of all captured arguments
	template <
		typename    t_Fn, // must be callable with t_Args
		typename... t_Args
	>
	constexpr auto curry(
		t_Fn      callable,
		t_Args... initial_args // may be zero args as well
	) {
		// if we can call the function with the full set of arguments, do so
		if constexpr (requires { std::invoke(callable, initial_args...); }) {
			return std::invoke(callable, initial_args...);
		}
		else {
			// otherwise, capture the arguments that were passed and yield a lambda that takes the remaining args
			return [callable, initial_args...](auto... extra_args) {
				return curry(callable, initial_args..., extra_args...);
			};
		}
	}

	// NOTE a weird interaction with multiple parameter packs... consider
	// 
	// template <typename...Ts, typename...Us>
	// int some_fn(Ts... ts, Us... us);
	//
	// here, the Ts are *only* matched against explicit parameters
	// and the Us are *greedily* deduced in a right-to-left fashion
	//
	// e.g.
	//		some_fn          (1, 2, 3); // Ts is <>, Us is <int, int, int>
	//      some_fn<int, int>(1, 2, 3); // Ts is <int, int>, Us is <int>
	//
	// BUT there is some weird interaction between the matching rules possible
	//
	// template <typename...Ts, typename... Us, typename V>
	// int corner_case(Ts..., Us..., V);
	//
	// e.g.
	//      corner_case     (1);       // Ts is <>, Us is <>, 
	//      corner_case     (1, 2);    // (!) Ts is <int>, Us is <>, V is <int> ? illegal interaction, no diagnostic required :P
	//      corner_case<int>(1, 2, 3); // Ts is <int>, Us is <int>, V is <int>
}