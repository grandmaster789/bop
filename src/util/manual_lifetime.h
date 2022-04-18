#pragma once

namespace bop::util {
	// make object construction and destruction very explicit, but keep the
	// automatic memory allocations possible.
	template <typename T>
	class ManualLifetime final {
	public:
		template <typename...t_Args>
		void construct(t_Args&&... args);
		void destroy();

		      T&  get()      &;
		const T&  get() const&;
		      T&& get()      &&;
		const T&& get() const&&;

	private:
		union {
			T m_Object;
		};
	};

	// void specialization
	template <>
	class ManualLifetime<void> {
	public:
		void construct() noexcept;
		void destroy()   noexcept;
		void get() const noexcept;
	};

	// reference specialization (non-owning)
	template <typename T>
	class ManualLifetime<T&> {
	public:
		ManualLifetime() noexcept;

		void construct(T& value) noexcept;
		void destroy()           noexcept;
		T&   get()         const noexcept;

	private:
		T* m_Pointer;
	};

	// rvalue specialization (non-owning)
	template <typename T>
	class ManualLifetime<T&&> {
	public:
		ManualLifetime() noexcept;

		void construct(T&& value) noexcept;
		void destroy()            noexcept;
		T&&  get()          const noexcept;

	private:
		T* m_Pointer;
	};
}

#include "manual_lifetime.inl"