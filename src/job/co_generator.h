#pragma once

#include <coroutine>
#include <optional>

namespace bop::job {
	// single-threaded wrapper around an algorithm that co_yields a T 
	// (see generator_range for an algorithm sample, unit test for usage)
	template <std::movable T>
	class Generator {
	public:
		struct promise_type {
			Generator<T> get_return_object();
			
			std::suspend_always yield_value(T new_value) noexcept;

			static std::suspend_always initial_suspend();
			static std::suspend_always final_suspend() noexcept;

			void        await_transform() = delete; // disable co_await
			static void return_void() noexcept; // promise type must declare return_void or return_value
			static void unhandled_exception();

			std::optional<T> m_CurrentValue;
		};

		// https://en.cppreference.com/w/cpp/coroutine/coroutine_handle
		using Handle = std::coroutine_handle<promise_type>;

		Generator() = default;
		explicit Generator(const Handle coro);
		~Generator();

		Generator             (const Generator&)     = delete;
		Generator& operator = (const Generator&)     = delete;
		Generator             (Generator&& g) noexcept;
		Generator& operator = (Generator&& g) noexcept;

		// range-based for loop support
		class Iter {
		public:
			explicit Iter(const Handle generator_coroutine);

			void     operator ++();
			const T& operator * () const;
			bool     operator ==(std::default_sentinel_t) const;

		private:
			Handle m_Coroutine;
		};

		Iter                    begin();
		std::default_sentinel_t end();

	private:
		Handle m_Coroutine;
	};

	template <std::integral I>
	Generator<I> generator_range(I first, I last, I step = 1);
}

#include "co_generator.inl"