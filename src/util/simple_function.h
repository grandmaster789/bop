#pragma once

#include <functional>

#include "thread_types.h"
#include "concepts.h"

namespace bop::util {
	// wraps void() functions
	class SimpleFunction {
	public:
		// Thread attributes may be adjusted after construction
		ThreadIndex m_ThreadIndex;
		ThreadType  m_ThreadType;
		ThreadID    m_ThreadID;

		SimpleFunction() = default;

		template <c_invocable t_Fn>
		SimpleFunction(t_Fn&& fn):
			m_Function(std::forward<t_Fn>(fn)) 
		{
		}

		void operator()() const;

		auto getFunction() &;
		auto getFunction() &&;

	private:
		std::function<void()> m_Function = []{};
	};

	template <typename T> concept c_simple_function = std::is_same_v<std::decay_t<T>, SimpleFunction>;
	template <typename T> concept c_std_function    = std::is_convertible_v<std::decay_t<T>, std::function<void()>>;
	template <typename T> concept c_functor         = c_simple_function<T> || c_std_function<T>;

	using void_function_ptr = void(*)();
}