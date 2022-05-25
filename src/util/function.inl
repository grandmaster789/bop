#pragma once

#include "function.h"

namespace bop::util {
	template <typename R, typename...Ts, size_t N>
	Function<R(Ts...), N>::~Function() {
		if (m_Controller)
			m_Controller(&m_StoredFn, nullptr, e_SpecialOperation::destroy);
	}

	template <typename R, typename...Ts, size_t N>
	Function<R(Ts...), N>::Function(const Function& fn) {
		if (fn) {
			fn.clone_data(m_StoredFn);

			m_Invoker    = fn.m_Invoker;
			m_Controller = fn.m_Controller;
		}
	}

	template <typename R, typename...Ts, size_t N>
	Function<R(Ts...), N>& Function<R(Ts...), N>::operator=(const Function& fn) {
		if (*this)
			destroy_data();

		if (fn) {
			fn.clone_data(m_StoredFn);

			m_Invoker    = fn.m_Invoker;
			m_Controller = fn.m_Controller;
		}
		else {
			m_Invoker    = nullptr;
			m_Controller = nullptr;
		}

		return *this;
	}

	template <typename R, typename...Ts, size_t N>
	Function<R(Ts...), N>::Function(Function&& fn) noexcept {
		swap(std::move(fn));
	}

	template <typename R, typename...Ts, size_t N>
	Function<R(Ts...), N>& Function<R(Ts...), N>::operator=(Function&& fn) noexcept {
		swap(std::move(fn));

		return *this;
	}

	template <typename R, typename...Ts, size_t N>
	Function<R(Ts...), N>::Function(std::nullptr_t) noexcept {
	}

	template <typename R, typename...Ts, size_t N>
	Function<R(Ts...), N>& Function<R(Ts...), N>::operator = (std::nullptr_t) noexcept {
		if (*this) {
			destroy_data();

			m_Invoker    = nullptr;
			m_Controller = nullptr;
		}

		return *this;
	}

	template <typename R, typename...Ts, size_t N>
	template <typename Fn>
	Function<R(Ts...), N>::Function(Fn&& fn) {
		using decayed = typename std::decay_t<Fn>;

		static_assert(alignof(decayed) <= alignof(Storage), "Invalid alignment");
		static_assert(sizeof(decayed)  <= sizeof(Storage),  "Requires too much storage");

		new (&m_StoredFn) decayed(std::forward<Fn>(fn)); // placement new to fill storage
		m_Invoker    = &invoke<decayed>;
		m_Controller = &control<decayed>;
	}

	template <typename R, typename...Ts, size_t N>
	template <typename Fn>
	Function<R(Ts...), N>& Function<R(Ts...), N>::operator = (Fn&& fn) {
		Function(std::forward<Fn>(fn)).swap(*this);
		return *this;
	}

	template <typename R, typename...Ts, size_t N>
	template <typename Fn>
	Function<R(Ts...), N>& Function<R(Ts...), N>::operator = (std::reference_wrapper<Fn> fn) {
		Function(fn).swap(*this);
		return *this;
	}

	template <typename R, typename...Ts, size_t N>
	R Function<R(Ts...), N>::operator()(Ts&&...args) {
		if (!m_Invoker)
			throw std::bad_function_call();

		return m_Invoker(&m_StoredFn, std::forward<Ts>(args)...);
	}

	template <typename R, typename...Ts, size_t N>
	[[nodiscard]] Function<R(Ts...), N>::operator bool() const noexcept {
		return !!m_Controller;
	}

	template <typename R, typename...Ts, size_t N>
	void Function<R(Ts...), N>::swap(Function& fn) noexcept {
		std::swap(m_Invoker,    fn.m_Invoker);
		std::swap(m_Controller, fn.m_Controller);
		std::swap(m_StoredFn,   fn.m_StoredFn);
	}

	template <typename R, typename...Ts, size_t N>
	template <typename Fn>
	R Function<R(Ts...), N>::invoke(void* stored_fn, Ts&&... args) {
		return (*static_cast<Fn*>(stored_fn))(std::forward<Ts>(args)...);
	}

	template <typename R, typename...Ts, size_t N>
	template <typename Fn>
	void Function<R(Ts...), N>::control(void* src, void* dst, e_SpecialOperation op) {
		switch (op) {
		case e_SpecialOperation::clone: 
			new (dst) Fn(*static_cast<Fn*>(src)); // use placement new to make a copy of the stored function
			break;

		case e_SpecialOperation::destroy: 
			static_cast<Fn*>(src)->~Fn();
			break;
		}
	}

	template <typename R, typename...Ts, size_t N>
	void Function<R(Ts...), N>::clone_data(Storage* dest) {
		m_Controller(m_StoredFn, dest, e_SpecialOperation::clone);
	}

	template <typename R, typename...Ts, size_t N>
	void Function<R(Ts...), N>::destroy_data() {
		m_Controller(m_StoredFn, nullptr, e_SpecialOperation::destroy);
	}
}