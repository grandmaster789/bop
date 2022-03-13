#pragma once

namespace bop::util {
	template <
		typename T,
		T t_NumElements, // NOTE consider making this a size_t instead of T
		T t_StartValue,
		T t_Cursor,      // NOTE this too may be a size_t
		T t_Step,
		T... t_Values
	> 
	struct ValueSequence;
	
	namespace detail {
		template <typename, size_t, size_t, typename...>
		struct VOEHelper;
	}

	template <size_t t_Index, typename T, typename... t_Values> 
	struct ValueOfElement {
		static_assert(t_Index < sizeof...(t_Values) || sizeof...(t_Values) == 0, "index out of range");
		static constexpr T value = detail::VOEHelper<t_Index, T, t_Values...>::value;
	};

	template <size_t t_Index, typename T, typename... t_Values>
	constexpr inline T value_of_element_v = ValueOfElement<t_Index, T, t_Values...>::value;

	template <typename...> 
	struct MergeValueLists;

	template <typename...Ts>
	using merge_value_lists_t = typename MergeValueLists<Ts...>::Type;

	template <typename T, T... t_Values>
	struct ValueList {
		static constexpr size_t k_Size = sizeof...(t_Values);

		template <size_t index>
		static constexpr T get = ValueOfElement<T, t_Values...>::value;
	};

	template <typename T, size_t t_Count, T t_Start = 0, T t_Step = 1>
	using make_value_list = ValueSequence<T, t_Count, t_Start, 0, t_Step>::Type;

	template <size_t...t_Values>
	using IndexList = ValueList<size_t, t_Values...>;

	template <size_t t_Count, size_t t_Start = 0, size_t t_Step = 1>
	using make_index_list = make_value_list<size_t, t_Count, t_Start, t_Step>;
}

// interfacing with std tuple
namespace std {
	template <typename T, T... t_Values>
	struct tuple_size<::bop::util::ValueList<T, t_Values...>>:
		std::integral_constant<size_t, sizeof...(t_Values)>
	{
	};

	template <size_t t_Index, typename T, T... t_Values>
	constexpr T get(::bop::util::ValueList<T, t_Values...>) noexcept {
		return ::bop::util::value_of_element_v<t_Index, T, t_Values...>;
	}
}

#include "valuelist.inl"