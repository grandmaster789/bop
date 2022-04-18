#pragma once

#include <coroutine>
#include <optional>

namespace bop::job {
	// single-threaded wrapper around an algorithm that co_yields a T 
	// [NOTE] a coroutine handle is a pointer
	// 
	// (see iota for an algorithm sample, unit test for usage)
	// effectively, this ends up resembling C# enumerable interfaces
	template <std::movable T>
	class Generator {
	public:
		struct promise_type final {
			Generator<T> get_return_object();
			
			std::suspend_always yield_value(const T& new_value) noexcept;
			std::suspend_always yield_value(T&& new_value) noexcept;

			static std::suspend_always initial_suspend();
			static std::suspend_always final_suspend() noexcept;

			void        await_transform() = delete; // disable co_await
			void        return_void() noexcept; // after the coroutine is done, clear the promise contents
			static void unhandled_exception();

			std::optional<T> m_CurrentValue;
		};

		// https://en.cppreference.com/w/cpp/coroutine/coroutine_handle
		using Handle = std::coroutine_handle<promise_type>;

		explicit Generator(promise_type* caller) noexcept;

		Generator() noexcept = default;
		~Generator() noexcept;

		Generator             (const Generator&) = delete;
		Generator& operator = (const Generator&) = delete;
		Generator             (Generator&& g) noexcept;
		Generator& operator = (Generator&& g) noexcept;

		// STL + range-based for loop support
		class Iter final {
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type   = std::ptrdiff_t;
			using value_type        = T;
			using reference         =       value_type&;
			using const_reference   = const value_type&;
			using pointer           =       value_type*;
			using const_pointer     = const value_type*;

			explicit Iter(const Handle generator_coroutine);

			// only allow post-increment
			Iter& operator++(int) = delete; 
			Iter& operator++();

			pointer         operator ->() const noexcept;
			const_reference operator * () const noexcept;

			bool operator ==(std::default_sentinel_t) const noexcept;
			bool operator ==(const Iter& it)          const noexcept;
			bool operator !=(const Iter& it)          const noexcept;

		private:
			Handle m_Coroutine = nullptr; // non-owning handle
		};

		Iter                    begin();
		std::default_sentinel_t end();

	private:
		Handle m_Coroutine = nullptr; // owning handle
	};

	template <std::integral I>
	Generator<I> iota(I up_to);

	template <std::integral I>
	Generator<I> range(I first, I last, I step = 1);
}

#include "co_generator.inl"