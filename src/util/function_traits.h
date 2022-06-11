#pragma once

#include <tuple>

namespace bop::util {
	template <typename T, typename = void> 
	struct IsCallable:
		std::false_type 
	{};

	// functor/lambda
	// [NOTE] does this also work when the functor has overloads?
	template <typename T>
	struct IsCallable<T, std::void_t<decltype(&T::operator())>> : 
		std::true_type  
	{};

	// global function
	template <typename R, typename... Args>
	struct IsCallable<R(*)(Args...)> : 
		std::true_type 
	{};

	// (mutable) member function
	template <typename T, typename R, typename... Args> 
	struct IsCallable<R(T::*)(Args...)> : 
		std::true_type 
	{};

	// const member function
	template <typename T, typename R, typename... Args> 
	struct IsCallable<R(T::*)(Args...) const> :
		std::true_type 
	{};

	// as a concept
	template <typename T>
	concept c_is_callable = IsCallable<T>::value;

	// traits should refer to the wrapped function
	// (default to the functor style)
	template <c_is_callable T>
	struct FunctionTraits :
		FunctionTraits<decltype(&T::operator())>
	{};

	// (mutable) member function (forwards to global function)
	template <typename T, typename R, typename... Args>
	struct FunctionTraits<R(T::*)(Args...)> :
		FunctionTraits<R(*)(Args...)> 
	{};

	// const member function (forwards to global function)
	template <typename T, typename R, typename... Args>
	struct FunctionTraits<R(T::*)(Args...) const> :
		FunctionTraits<R(*)(Args...)>
	{};

	// zero argument global function
	template <typename R>
	struct FunctionTraits<R(*)()> {
		using Result = R;

		static constexpr size_t k_NumArgs = 0;

		template <template <typename> class t_Lift>
		using LiftResult = t_Lift<R>;
	};

	struct Empty {}; // this should probably be somewhere shared

	// global function
	template <typename R, typename... Args>
	struct FunctionTraits<R(*)(Args...)> {
		using Result    = R;
		using Arguments = std::tuple<Args...>; // wrap types into a tuple

		static constexpr size_t k_NumArgs = sizeof...(Args);

		template <size_t Idx = 0>
			requires (k_NumArgs > Idx)
		using Arg = std::tuple_element_t<Idx, Arguments>;

		template <template <typename> class t_Lift>
		using LiftResult = t_Lift<R>;

		template <template <typename> class t_Lift>
		using LiftArguments = std::tuple<t_Lift<Args>...>;
	};

	// void overload #1
	template <typename...Args>
	struct FunctionTraits<void(*)(Args...)> {
		using Result = void;
		using Arguments = std::tuple<Args...>;

		static constexpr size_t k_NumArgs = sizeof...(Args);

		template <size_t Idx = 0>
			requires (k_NumArgs > Idx)
		using Arg = std::tuple_element_t<Idx, Arguments>;

		template <template <typename> class>
		using LiftResult = Empty;

		template <template <typename> class t_Lift>
		using LiftArguments = std::tuple<t_Lift<Args>...>;
	};

	// void overload #2
	template <>
	struct FunctionTraits<void(*)()> {
		using Result = void;
		using Arguments = std::tuple<>; // still wrap types into a tuple; this allows tuple-based algorithms to work

		static constexpr size_t k_NumArgs = 0;

		template <template <typename> class>
		using LiftResult = Empty;

		template <template <typename> class>
		using LiftArguments = std::tuple<>; // still yield a tuple; this allows tuple-based algorithms to work
	};

	template <typename T, size_t Idx = 0>
	using FunctionArg = typename FunctionTraits<T>::template Arg<Idx>;

	template <typename T, size_t Idx = 0>
	using BasicFunctionArg = std::remove_cv_t<FunctionArg<T, Idx>>;

	template <typename T, template <typename> typename t_Lift>
	using LiftResult = typename FunctionTraits<T>::template LiftResult<t_Lift>;

	template <typename T, template <typename> typename t_Lift>
	using LiftArguments = typename FunctionTraits<T>::template LiftArguments<t_Lift>;
}