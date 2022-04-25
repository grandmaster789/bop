#pragma once

#include "co_job.h"

namespace bop::job {
	// CoJob<T>
	template <typename T>
	CoJob<T>::CoJob(promise_type* pm) noexcept:
		m_Handle(Handle::from_promise(*pm))
	{
	}

	template <typename T>
	CoJob<T>::~CoJob() {
		if (m_Handle)
			m_Handle.destroy();
	}

	template <typename T>
	bool CoJob<T>::await_ready() noexcept {
		return false;
	}

	template <typename T>
	void CoJob<T>::await_suspend(Handle) noexcept {
	}

	template <typename T>
	T CoJob<T>::await_resume() {
		// this will work for 'returning' coroutines, but not for 'yielding' coroutines
		while (m_Handle && !m_Handle.done())
			m_Handle.resume();

		auto& pm = m_Handle.promise();

		if (auto* exp = std::get_if<std::exception_ptr>(&pm.m_Payload))
			std::rethrow_exception(*exp);
		
		if (pm.m_Payload.index() == 0) [[unlikely]]
			throw std::runtime_error("Job promise is in empty state");
		
		return std::get<T>(pm.m_Payload);
	}

	template <typename T>
	T CoJob<T>::get() {
		return await_resume();
	}

	// CoJob<void>
	inline CoJob<void>::CoJob(promise_type* pm) noexcept:
		m_Handle(Handle::from_promise(*pm))
	{
	}

	inline CoJob<void>::~CoJob() {
		if (m_Handle)
			m_Handle.destroy();
	}

	inline bool CoJob<void>::await_ready() noexcept {
		return false;
	}

	inline void CoJob<void>::await_suspend(Handle) noexcept {
	}

	inline void CoJob<void>::await_resume() {
		while (m_Handle && !m_Handle.done())
			m_Handle.resume();

		auto& pm = m_Handle.promise();

		if (pm.m_Payload)
			std::rethrow_exception(*pm.m_Payload);
	}

	inline void CoJob<void>::get() {
		await_resume();
	}

	// CoJobPromise<void>
	inline CoJob<void> CoJobPromise<void>::get_return_object() {
		return CoJob<void>(this);
	}

	inline void CoJobPromise<void>::return_void() {
	}

	inline void CoJobPromise<void>::unhandled_exception() {
		m_Payload = std::current_exception();
	}

	inline auto CoJobPromise<void>::initial_suspend() {
		return std::suspend_always();
	}

	inline auto CoJobPromise<void>::final_suspend() noexcept {
		return std::suspend_always();
	}
}