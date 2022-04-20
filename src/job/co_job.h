#pragma once

#include <coroutine>
#include "co_job_promise.h"

namespace bop::job {
	template <typename T = void>
	class CoJob {
	public:
		using promise_type = CoJobPromise<T>;
		using Handle       = std::coroutine_handle<promise_type>;

		explicit CoJob(promise_type* pm) noexcept;
		~CoJob();

		bool await_ready() noexcept;
		void await_suspend(Handle parent) noexcept;
		T    await_resume();

		T    get();

	private:
		Handle m_Handle;
	};

	// void specialization
	template <>
	class CoJob<void> {
	public:
		using promise_type = CoJobPromise<void>;
		using Handle       = std::coroutine_handle<promise_type>;

		explicit CoJob(promise_type* pm) noexcept;
		~CoJob();

		bool await_ready() noexcept;
		void await_suspend(Handle) noexcept;
		void await_resume();

		void get();
		
	private:
		Handle m_Handle;
	};
}

#include "co_job.inl"