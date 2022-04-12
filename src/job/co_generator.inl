#include "co_generator.h"

namespace bop::job {
	/***** promise_type *****/
	template <typename T>
	Generator<T> Generator<T>::promise_type::get_return_object() {
		return Generator{ Handle::from_promise(*this) };
	}

	template <typename T>
	std::suspend_always Generator<T>::promise_type::initial_suspend() {
		return {};
	}

	template <typename T>
	std::suspend_always Generator<T>::promise_type::final_suspend() noexcept {
		return {};
	}

	template <typename T>
	std::suspend_always Generator<T>::promise_type::yield_value(T new_value) {
		m_CurrentValue = std::forward<T>(new_value);
		return {};
	}

	template <typename T>
	void Generator<T>::promise_type::unhandled_exception() {
	}

	template <typename T>
	void Generator<T>::promise_type::return_void() {
	}

	/***** Generator *****/
	template <typename T>
	Generator<T>::Generator(promise_type::Handle coro):
		m_Coroutine(coro)
	{
	}

	template <typename T>
	Generator<T>::~Generator() {
		if (m_Coroutine)
			m_Coroutine.destroy();
	}

	template <typename T>
	T Generator<T>::get_next() {
		m_Coroutine.resume();
		return m_Coroutine.promise().m_CurrentValue;
	}
}