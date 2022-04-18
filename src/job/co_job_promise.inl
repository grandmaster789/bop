#pragma once

#include "co_job_promise.h"
#include "co_job.h"
#include "../util/overloaded.h"

#include <stdexcept>

namespace bop::job {
	template <std::movable T>
	CoJob<T> CoJobPromise<T>::get_return_object() noexcept {
		return CoJob<T>(this);
	}

	template <std::movable T>
	std::suspend_always CoJobPromise<T>::initial_suspend() {
		return {};
	}

	template <std::movable T>
	auto CoJobPromise<T>::final_suspend() noexcept {
		struct Awaiter {
			bool await_ready() noexcept {
				return false;
			}

			auto await_suspend(std::coroutine_handle<CoJobPromise> handle) noexcept {
				return handle.promise().m_Continuation;
			}

			void await_resume() noexcept {
			}
		};

		return Awaiter();
	}

	template <std::movable T>
	template <typename U>
	requires std::convertible_to<U, T>
	void CoJobPromise<T>::return_value(U&& set_result) {
		m_State = std::forward<U>(set_result);
	}

	template <std::movable T>
	void CoJobPromise<T>::unhandled_exception() noexcept {
		m_State = std::current_exception();
	}

	template <std::movable T>
	T CoJobPromise<T>::get() {
		std::visit(util::Overloaded{
			[](auto) { throw std::runtime_error("No value is set"); },
			[](std::exception_ptr x) { std::rethrow_exception(x); },
			[](T value) { return value; }
		}, m_State);
	}

	// void specialization
	inline CoJob<void> CoJobPromise<void>::get_return_object() noexcept {
		return CoJob<void>(this);
	}

	inline std::suspend_always CoJobPromise<void>::initial_suspend() {
		return {};
	}

	inline auto CoJobPromise<void>::final_suspend() noexcept {
		struct Awaiter {
			bool await_ready() noexcept {
				return false;
			}

			auto await_suspend(std::coroutine_handle<CoJobPromise> handle) noexcept {
				return handle.promise().m_Continuation;
			}

			void await_resume() {
			}
		};

		return Awaiter();
	}

	inline void CoJobPromise<void>::return_void() noexcept {
		// so technically we need a 'value' state here, but the promise is of void type...
		m_State = VoidValue();
	}

	inline void CoJobPromise<void>::unhandled_exception() noexcept {
		m_State = std::current_exception();
	}

	inline void CoJobPromise<void>::get() {
		std::visit(util::Overloaded{
			[](std::exception_ptr x) { std::rethrow_exception(x); },
			[](auto) {}
		}, m_State);
	}
}