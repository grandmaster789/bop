#pragma once

#include "co_job.h"

namespace bop::job {
	template <typename T>
	CoJob<T>::CoJob(promise_type* promise):
		m_Coroutine(Handle::from_promise(*promise))
	{
	}

	template <typename T>
	CoJob<T>::~CoJob() {
		if (m_Coroutine)
			m_Coroutine.destroy();
	}

	template <typename T>
	auto CoJob<T>::operator co_await() && noexcept {
		struct Awaiter {
			Handle m_Coro;

			explicit Awaiter(Handle coro):
				m_Coro(coro)
			{
			}

			bool await_ready() {
				return false;
			}

			auto await_suspend(std::coroutine_handle<> handle) noexcept {
				m_Coro.promise().m_Continuation = handle;
				return m_Coro;
			}

			T await_resume() {
				return m_Coro.promise().get();
			}
		};


		return Awaiter(m_Coroutine);
	}
}