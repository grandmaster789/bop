#pragma once

#include "valuelist.h"
#include "typelist.h"

#include <tuple>

namespace bop::util {
	namespace detail {
		// Value of Element Helper
		template <
			typename,   // value_type
			size_t,     // index
			size_t,     // cursor
			typename... // value sequence
		>
		struct VOEHelper;

		template <
			typename T,
			size_t t_Index,
			size_t t_Cursor,
			typename t_Head,
			typename... t_Tail
		>
			struct VOEHelper<T, t_Index, t_Cursor, t_Head, t_Tail...> {
			static constexpr T value = VOEHelper<T, t_Index, t_Cursor + 1ull, t_Tail...>::value;
		};

		template <
			typename T,
			size_t t_Index,
			typename t_Head,
			typename... t_Tail
		>
			struct VOEHelper<T, t_Index, t_Index, t_Head, t_Tail...> {
			static constexpr T value = t_Head;
		};
	}

	// value sequence
	template <
		typename T,
		T t_NumElements,
		T t_StartingValue,
		T t_Cursor,
		T t_Step,
		T... t_Values
	>
	struct ValueSequence {
		using Type = typename ValueSequence<
			T,
			t_NumElements,
			t_StartingValue,
			t_Cursor + 1, // advance the cursor
			t_Step,
			t_Values...,
			t_StartingValue + t_Step * sizeof...(t_Values) // append the next value
		>::Type;
	};

	template <
		typename T, 
		T t_NumElements,
		T t_StartingValue,
		T t_Step,
		T... t_Values
	>
	struct ValueSequence<T, t_NumElements, t_StartingValue, t_NumElements, t_Step, t_Values...> {
		using Type = ValueList<T, t_Values...>;
	};

	// merge value list
	template <
		typename T,
		T... t_ValuesA,
		T... t_ValuesB,
		typename... t_Additional
	>
	struct MergeValueLists<
		ValueList<T, t_ValuesA...>,
		ValueList<T, t_ValuesB...>,
		t_Additional...
	> {
		using Type = typename MergeValueLists<ValueList<T, t_ValuesA..., t_ValuesB...>, t_Additional...>;
	};

	template <
		typename T,
		T... t_ValuesA,
		T... t_ValuesB
	>
	struct MergeValueLists<
		ValueList<T, t_ValuesA...>,
		ValueList<T, t_ValuesB...>
	> {
		using Type = ValueList<T, t_ValuesA..., t_ValuesB...>;
	};
}