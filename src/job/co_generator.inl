#include "co_generator.h"

namespace bop::job {
	/***** promise_type *****/
	template <std::movable T>
	Generator<T> Generator<T>::promise_type::get_return_object() {
		return Generator(this);
	}

	template <std::movable T>
	std::suspend_always Generator<T>::promise_type::yield_value(const T& new_value) noexcept {
		m_CurrentValue = new_value;
		return {};
	}

	template <std::movable T>
	std::suspend_always Generator<T>::promise_type::yield_value(T&& new_value) noexcept {
		m_CurrentValue = std::forward<T>(new_value);
		return {};
	}

	template <std::movable T>
	std::suspend_always Generator<T>::promise_type::initial_suspend() {
		return {};
	}

	template <std::movable T>
	std::suspend_always Generator<T>::promise_type::final_suspend() noexcept {
		return {};
	}	

	template <std::movable T>
	void Generator<T>::promise_type::return_void() noexcept {
		m_CurrentValue = std::nullopt;
	}

	template <std::movable T>
	void Generator<T>::promise_type::unhandled_exception() {
		throw; // rethrow exception
	}

	/***** Generator *****/
	template <std::movable T>
	Generator<T>::Generator(promise_type* caller) noexcept:
		m_Coroutine(Handle::from_promise(*caller))
	{
	}

	template <std::movable T>
	Generator<T>::~Generator() noexcept {
		if (m_Coroutine)
			m_Coroutine.destroy();
	}

	template <std::movable T>
	Generator<T>::Generator(Generator&& g) noexcept:
		m_Coroutine(g.m_Coroutine)
	{
		g.m_Coroutine = nullptr;
	}

	template <std::movable T>
	Generator<T>& Generator<T>::operator = (Generator&& g) noexcept {
		if (&g != this) {
			if (m_Coroutine)
				m_Coroutine.destroy();

			m_Coroutine = g.m_Coroutine;
			g.m_Coroutine = nullptr;
		}

		return *this;
	}

	template <std::movable T>
	Generator<T>::Iter Generator<T>::begin() {
		if (m_Coroutine)
			m_Coroutine.resume();

		return Iter(m_Coroutine);
	}

	template <std::movable T>
	std::default_sentinel_t Generator<T>::end() {
		return {};
	}

	/***** Iter *****/
	template <std::movable T>
	Generator<T>::Iter::Iter(const Handle generator_coroutine):
		m_Coroutine(generator_coroutine)
	{
	}

	template <std::movable T>
	Generator<T>::Iter& Generator<T>::Iter::operator ++() {
		m_Coroutine.resume();

		if (m_Coroutine.done())
			m_Coroutine = nullptr;

		return *this;
	}

	template <std::movable T>
	Generator<T>::Iter::pointer Generator<T>::Iter::operator ->() const noexcept {
		auto& opt = m_Coroutine.promise().m_CurrentValue;

		if (opt)
			return &*opt;
		else
			return nullptr;
	}
	
	template <std::movable T>
	Generator<T>::Iter::const_reference Generator<T>::Iter::operator *() const noexcept {
		return *(m_Coroutine.promise().m_CurrentValue);
	}

	template <std::movable T>
	bool Generator<T>::Iter::operator ==(std::default_sentinel_t) const noexcept {
		return
			!m_Coroutine ||
			m_Coroutine.done();
	}

	template <std::movable T>
	bool Generator<T>::Iter::operator==(const Iter& it) const noexcept {
		return m_Coroutine == it.m_Coroutine;
	}

	template <std::movable T>
	bool Generator<T>::Iter::operator!=(const Iter& it) const noexcept {
		return !(*this == it);
	}

	/***** generator functions *****/
	template <std::integral I>
	Generator<I> iota(I up_to) {
		for (I value = 0; value < up_to; ++value)
			co_yield value;
	}


	template <std::integral I>
	Generator<I> range(I first, I last, I step) {
		while (first < last) {
			co_yield first;
			first = first + step;
		}
	}
}
