#pragma once

#include <functional>

namespace bop::util {
	// from C++Weekly 318
	// very neat, but it does make a copy of all captured arguments
	template <
		typename    t_Fn, // must be callable with t_Args
		typename... t_Args
	>
	constexpr decltype(auto) curry(
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
}