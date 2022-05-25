#pragma once

#include <functional>
#include <memory>

namespace bop::util {
	template <typename t_Fn, size_t t_MaxSize = 64> // not sure what a sensible maximum size is
	class Function;

	template <
		typename    t_Result, 
		typename... t_Args, 
		size_t      t_MaxSize
	>
	class Function<t_Result(t_Args...), t_MaxSize> {
	public:
		Function() noexcept = default;
		~Function();

		Function             (const Function& fn);
		Function& operator = (const Function& fn);
		Function             (Function&& fn) noexcept;
		Function& operator = (Function&& fn) noexcept;

		Function             (std::nullptr_t) noexcept;
		Function& operator = (std::nullptr_t) noexcept;

		template <typename Fn> Function             (Fn&& fn);
		template <typename Fn> Function& operator = (Fn&& fn);
		template <typename Fn> Function& operator = (std::reference_wrapper<Fn> fn);

		[[nodiscard]] explicit operator bool() const noexcept;

		void swap(Function& fn) noexcept;

		t_Result operator()(t_Args&&... args);

	private:
		// this is used so that 'special' operations can be done with a single function pointer
		// -- a 'controller' function to perform the special operations
		enum class e_SpecialOperation {
			clone,
			destroy
		};

		template <typename Fn> static t_Result invoke(void* stored_fn, t_Args&&... args);
		template <typename Fn> static void     control(void* src, void* dst, e_SpecialOperation op);

		using Invoker    = t_Result(*)(void*, t_Args&&...);
		using Controller = void(*)(void*, void*, e_SpecialOperation);
		using Storage    = typename std::aligned_storage_t<
			t_MaxSize - sizeof(Invoker) - sizeof(Controller), 
			8 // this is kind-of platform specific
		>;

		void clone_data  (Storage* destination);
		void destroy_data();

		Invoker    m_Invoker    = nullptr;
		Controller m_Controller = nullptr;
		Storage    m_StoredFn;
	};

	
}

#include "function.inl"