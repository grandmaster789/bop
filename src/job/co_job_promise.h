#pragma once

#include <coroutine>
#include <atomic>
#include <exception>
#include <optional>
#include <variant>

namespace bop::job {
	template <typename T>
	class CoJob;

	template <typename T>
	class CoJobPromise;

	// most jobs should use this
	template <std::movable T>
	class CoJobPromise<T> final {
	public:
		friend class CoJob<T>;

		CoJob<T> get_return_object() noexcept;

		std::suspend_always initial_suspend();
		auto                final_suspend() noexcept;

		template <typename U>
		requires std::convertible_to<U, T>
		void return_value(U&& set_result);

		void unhandled_exception() noexcept;

		T get();

	private:
		std::coroutine_handle<>                             m_Continuation; // type-erased
		std::variant<std::monostate, T, std::exception_ptr> m_State;
	};

	// void specialization
	template <>
	class CoJobPromise<void> final {
	public:
		friend class CoJob<void>;

		CoJob<void> get_return_object() noexcept;

		std::suspend_always initial_suspend();
		auto                final_suspend() noexcept;

		void return_void() noexcept;
		void unhandled_exception() noexcept;

		void get();

	private:
		struct VoidValue {};

		std::coroutine_handle<>                                     m_Continuation; // type-erased
		std::variant<std::monostate, VoidValue, std::exception_ptr> m_State = std::monostate();
	};
}

#include "co_job_promise.inl"