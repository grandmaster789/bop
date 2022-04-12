#pragma once

#include <coroutine>

namespace bop::job {
	template <typename T>
	class Generator {
	public:
		struct promise_type {
			using Handle = std::coroutine_handle<promise_type>;

			Generator<T> get_return_object();

			std::suspend_always initial_suspend();
			std::suspend_always final_suspend() noexcept;
			std::suspend_always yield_value(T new_value);

			void unhandled_exception(); // doesn't handle exceptions
			void return_void();

			T m_CurrentValue;
		};

		explicit Generator(promise_type::Handle coro);
		~Generator();

		Generator             (const Generator&)     = delete;
		Generator& operator = (const Generator&)     = delete;
		Generator             (Generator&&) noexcept = default;
		Generator& operator = (Generator&&) noexcept = default;

		T get_next(); // resumes the coroutine and returns the next value

	private:
		promise_type::Handle m_Coroutine;
	};
}

#include "co_generator.inl"