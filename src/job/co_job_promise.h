#pragma once

#include <coroutine>
#include <atomic>
#include <exception>

namespace bop::job::detail {
	class JobPromiseBase {
	private:
		friend struct FinalAwaitable;

		struct FinalAwaitable {
			constexpr bool await_ready() const noexcept;
			constexpr void await_resume() noexcept;

			template <typename T>
			void await_suspend(std::coroutine_handle<T> handle);
		};

	public:
		JobPromiseBase() noexcept = default;

		constexpr std::suspend_always initial_suspend() noexcept;
		constexpr FinalAwaitable      final_suspend() noexcept;

		constexpr void set_continuation(std::coroutine_handle<> continuation) noexcept;

	private:
		std::coroutine_handle<> m_Continuation;
		std::atomic<bool>       m_State = false;
	};
}

namespace bop::job {
	template <typename T>
	class JobPromise final:
		public detail::JobPromiseBase
	{
	public:
		JobPromise() noexcept = default;
		~JobPromise();

		//Job<T> get_return_object() noexcept;

		constexpr void unhandled_exception() noexcept;

	private:
		// tagged union

		enum class e_Result {
			empty,
			value,
			exception
		} m_Result = e_Result::empty;

		union {
			T                  m_Value;
			std::exception_ptr m_Exception;
		};
	};

	template <>
	class JobPromise<void> :
		public detail::JobPromiseBase
	{
	public:
		JobPromise() noexcept = default;

		//Job<void> get_return_object() noexcept;

		constexpr void return_void() noexcept {}
		void unhandled_exception() noexcept {
			m_Exception = std::current_exception();
		}

		void result() {
			if (m_Exception)
				std::rethrow_exception(m_Exception);
		}

	private:
		std::exception_ptr m_Exception;
	};
}

#include "co_job_promise.inl"