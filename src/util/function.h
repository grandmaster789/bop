#pragma once

#include <functional>

#include "thread_types.h"
#include "concepts.h"

namespace bop::util {
	// wraps void() functions
	class Function {
	public:
		// Thread attributes may be adjusted after construction
		ThreadIndex m_ThreadIndex;

		Function() = default;

		template <c_invocable t_Fn>
		Function(t_Fn&& fn):
			m_Function(std::forward<t_Fn>(fn)) 
		{
		}

		inline void operator()() const {
			m_Function();
		}

	private:
		std::function<void()> m_Function = []{};
	};

	template <typename T> concept c_bop_function = std::is_same_v<std::decay_t<T>, Function>;
	template <typename T> concept c_std_function = std::is_convertible_v<std::decay_t<T>, std::function<void()>>;
	template <typename T> concept c_functor      = c_bop_function<T> || c_std_function<T>;

	using void_function_ptr = void(*)();
}