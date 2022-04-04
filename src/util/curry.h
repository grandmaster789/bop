#pragma once

#include <functional>

namespace bop::util {
	// from C++Weekly 318
	template <
		typename    t_Fn, // must be callable with t_Args
		typename... t_Args
	>
	constexpr decltype(auto) curry(
		t_Fn      callable,
		t_Args... initial_args
	) {
		// if we can call the function with the full set of arguments, do so
		if constexpr (requires { std::invoke(callable, initial_args...); }) {
			return std::invoke(callable, initial_args...);
		}
		else {
			return [callable, initial_args...](auto... extra_args) {
				return curry(callable, initial_args..., extra_args...);
			};
		}
	}
}