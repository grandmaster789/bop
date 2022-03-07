#pragma once

#include <tuple>
#include <type_traits>
#include <cstdint>

namespace bop::util {
	// type list operations (type result in T::Type)
	template <typename...>                              struct TypeList;
	template <int64_t t_Index, typename... t_Types>     struct TypeOfElement;
	template <typename>                                 struct ExtractTypeList;
	template <typename...>                              struct MergeTypeLists;
	template <
		template <typename...> typename t_Template, 
		typename Typelist
	>                                                   struct InsertTypeList;
	template <typename T>                               struct RemoveFirstType;
	template <typename...>                              struct ReverseTypeList;
}

#include "typelist.inl"