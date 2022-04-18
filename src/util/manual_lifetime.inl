#pragma once

#include "manual_lifetime.h"
#include <utility>

namespace bop::util {
	template <typename T>
	template <typename...t_Args>
	void ManualLifetime<T>::construct(t_Args&&... args) {
		::new (static_cast<void*>(std::addressof(m_Object))) T(static_cast<t_Args&&>(args)...);
	}

	template <typename T>
	void ManualLifetime<T>::destroy() {
		m_Object.~T();
	}

	template <typename T>
	T& ManualLifetime<T>::get()& {
		return m_Object;
	}

	template <typename T>
	const T& ManualLifetime<T>::get() const& {
		return m_Object;
	}

	template <typename T>
	T&& ManualLifetime<T>::get()&& {
		return static_cast<T&&>(m_Object);
	}

	template <typename T>
	const T&& ManualLifetime<T>::get() const&& {
		return static_cast<const T&&>(m_Object);
	}

	// void specialization
	inline void ManualLifetime<void>::construct() noexcept {
	}

	inline void ManualLifetime<void>::destroy() noexcept {
	}

	inline void ManualLifetime<void>::get() const noexcept {
	}

	// reference specialization
	template <typename T>
	ManualLifetime<T&>::ManualLifetime() noexcept:
		m_Pointer(nullptr)
	{
	}

	template <typename T>
	void ManualLifetime<T&>::construct(T& value) noexcept {
		m_Pointer = std::addressof(value);
	}

	template <typename T>
	void ManualLifetime<T&>::destroy() noexcept {
	}

	template <typename T>
	T& ManualLifetime<T&>::get() const noexcept {
		return *m_Pointer;
	}

	// rvalue specialization
	template <typename T>
	ManualLifetime<T&&>::ManualLifetime() noexcept :
		m_Pointer(nullptr)
	{
	}

	template <typename T>
	void ManualLifetime<T&&>::construct(T&& value) noexcept {
		m_Pointer = std::addressof(value);
	}

	template <typename T>
	void ManualLifetime<T&&>::destroy() noexcept {
	}

	template <typename T>
	T&& ManualLifetime<T&&>::get() const noexcept {
		return *m_Pointer;
	}
}