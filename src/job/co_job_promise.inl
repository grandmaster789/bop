#pragma once

#include "co_job_promise.h"

namespace bop::job::detail {
	constexpr bool JobPromiseBase::FinalAwaitable::await_ready() const noexcept {
		return false;
	}

	constexpr void JobPromiseBase::FinalAwaitable::await_resume() noexcept {
	}

	template <typename T>
	void JobPromiseBase::FinalAwaitable::await_suspend(std::coroutine_handle<T> handle) {
		auto& promise = handle.promise();

		if (promise.m_State.exchange(true, std::memory_order_acq_rel))
			promise.m_Continuation.resume();
	}

	constexpr std::suspend_always JobPromiseBase::initial_suspend() noexcept {
		return {};
	}

	constexpr JobPromiseBase::FinalAwaitable JobPromiseBase::final_suspend() noexcept {
		return {};
	}

	constexpr void JobPromiseBase::set_continuation(std::coroutine_handle<> continuation) noexcept {
		m_Continuation = continuation;
	}
}

namespace bop::job {
	/**** JobPromise<T> ****/
	template <typename T>
	JobPromise<T>::~JobPromise() {
		// because this is a tagged union we'll manually have to call the appropriate destructor
		switch (m_Result) {
		case e_Result::empty:                                   return;
		case e_Result::value:     m_Value.~T();                 return;
		case e_Result::exception: m_Exception.~exception_ptr(); return;
		}
	}

	template <typename T>
	constexpr void JobPromise<T>::unhandled_exception() noexcept {
		// in-place construction of the exception ptr
		::new (static_cast<void*>(std::addressof(m_Exception))) std::exception_ptr(std::current_exception());
		m_Result = e_Result::exception;
	}
}