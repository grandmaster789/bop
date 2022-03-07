#pragma once

namespace bop::util {
	template <typename>
	struct Signature;

	template <typename t_Result, typename... t_Args>
	struct Signature<t_Result(t_Args...)> {
		using result_type = t_Result;
	};
}