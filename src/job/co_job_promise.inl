#pragma once

#include "co_job_promise.h"
#include <exception>

namespace bop::job {
	// CoJobPromise<T>
	template <typename T>
	CoJob<T> CoJobPromise<T>::get_return_object() {
		return CoJob<T>(this);
	}

	template <typename T>
	void CoJobPromise<T>::return_value(T value) {
		m_Payload = std::move(value);
	}

	template <typename T>
	void CoJobPromise<T>::unhandled_exception() {
		m_Payload = std::current_exception();
	}

	template <typename T>
	auto CoJobPromise<T>::initial_suspend() {
		return std::suspend_always();
	}

	template <typename T>
	auto CoJobPromise<T>::final_suspend() noexcept {
		return std::suspend_always();
	}
}