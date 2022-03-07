#pragma once

#include <concepts>

namespace bop::util {
	template <typename T, typename... t_Args>
	concept c_one_of =
		sizeof...(t_Args) > 0 &&
		((std::same_as<T, t_Args>) || ...);

	template <typename T>
	concept c_has_member_co_await = requires(T(*object)()) {
		object().operator co_await();
	};

	template <typename T>
	concept c_has_global_co_await = requires(T(*object)()) {
		operator co_await(object());
	};

	template <typename T>
	concept c_is_co_awaiter = requires(T object) {
		{ object.await_ready() } -> std::same_as<bool>;
		object.await_resume();

		// can we restrict this further?
	};

	template <typename T>
	concept c_awaitable =
		c_has_member_co_await<T> ||
		c_has_global_co_await<T> ||
		c_is_co_awaiter<T>;

	template <typename T>
	concept c_enumeration = std::is_enum_v<std::remove_cv_t<T>>;
}