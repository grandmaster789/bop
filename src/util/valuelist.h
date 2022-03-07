#pragma once

namespace bop::util {
	// value list operations (values in ::value, type results in ::Type)
	template <typename T, T... t_Values>                struct ValueList;
	template <
		typename T,
		T t_NumElements, // NOTE consider making this a size_t instead of T
		T t_StartValue,
		T t_Cursor,      // NOTE this too may be a size_t
		T t_Step,
		T... t_Values
	>                                                   struct ValueSequence;
	template <size_t, typename T, typename... t_Values> struct ValueOfElement;
	template <typename...>                              struct MergeValueLists;
}

#include "valuelist.inl"