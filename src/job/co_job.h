#pragma once

#include <coroutine>

namespace bop::job {
	template <typename T>
	class CoJobPromise;

	template <typename T>
	class CoJob {
	public:
		using promise_type = CoJobPromise<T>;
		using Handle = std::coroutine_handle<promise_type>;

		explicit CoJob(promise_type* promise);
		~CoJob();

		auto operator co_await() && noexcept;

	private:
		Handle m_Coroutine;
	};
}

#include "co_job.inl"