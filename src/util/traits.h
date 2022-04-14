#pragma once

#include <memory_resource>
#include <type_traits>
#include <coroutine>

#include "concepts.h"

namespace bop::util {
	// PMR-related
	template <typename  > struct         is_pmr_vector:                      std::false_type {};
	template <typename T> struct         is_pmr_vector<std::pmr::vector<T>>: std::true_type  {};
	template <typename T> constexpr bool is_pmr_vector_v = is_pmr_vector<T>::value;
	template <typename T> concept        c_is_pmr_vector = is_pmr_vector_v<T>;

	// coroutine_handle
	template <typename  > struct         is_coroutine_handle:                            std::false_type {};
	template <typename T> struct         is_coroutine_handle<std::coroutine_handle<T>> : std::true_type  {};
	template <typename T> constexpr bool is_coroutine_handle_v = is_coroutine_handle<T>::value;
	template <typename T> concept        c_is_coroutine_handle = is_coroutine_handle_v<T>;

	// await_suspend may either return void, bool or a coroutine handle
	template <typename  > struct         is_valid_await_suspend_return_value :                           std::false_type {};
	template <>           struct         is_valid_await_suspend_return_value<void> :                     std::true_type {};
	template <>           struct         is_valid_await_suspend_return_value<bool> :                     std::true_type {};
	template <typename T> struct         is_valid_await_suspend_return_value<std::coroutine_handle<T>> : std::true_type {};
	template <typename T> constexpr bool is_valid_await_suspend_return_value_v = is_valid_await_suspend_return_value<T>::value;
	template <typename T> concept        c_is_valid_await_suspend_return_value = is_valid_await_suspend_return_value_v<T>;

	// awaiter must have await_ready, await_resume and await_suspend
	template <typename T, typename U = void>
	concept c_is_awaiter = requires (T obj, std::coroutine_handle<U> handle) {
		{ obj.await_ready()         } -> c_convertible<bool>;
		{ obj.await_resume()        } -> c_same<void>;
		{ obj.await_suspend(handle) } -> c_is_valid_await_suspend_return_value;
	};
	template <typename,   typename = void>                          struct is_awaiter :      std::false_type {};
	template <typename T, typename U> requires (c_is_awaiter<T, U>) struct is_awaiter<T, U>: std::true_type  {};
	template <typename T, typename U = void> 
	constexpr bool is_awaiter_v = is_awaiter<T, U>::value;
}