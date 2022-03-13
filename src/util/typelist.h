#pragma once

#include <tuple>
#include <type_traits>
#include <cstdint>

namespace bop::util {
	// type list operations (type result in T::Type)
	template <size_t t_Index, typename... t_Types>
	struct TypeOfElement;

	template <size_t t_Index, typename... t_Types>
	using type_of_element_t = typename TypeOfElement<t_Index, t_Types...>::Type;

	template <typename> 
	struct ExtractTypeList;

	template <typename T>
	using extract_type_list_t = typename ExtractTypeList<T>::Type;

	template <typename...> 
	struct MergeTypeLists;

	template <typename... Ts>
	using merge_type_list_t = typename MergeTypeLists<Ts...>::Type;

	template <									    
		template <typename...> typename t_Template, 
		typename Typelist						    
	> struct InsertTypeList;

	template <template <typename...> typename t_Template, typename t_TypeList>
	using insert_type_list_t = typename InsertTypeList<t_Template, t_TypeList>::Type;

	template <typename T> 
	struct RemoveFirstType;

	template <typename T>
	using remove_first_type_t = typename RemoveFirstType<T>::Type;

	template <typename t_Head, typename... t_Tail>
	using first_t = t_Head;

	template <typename...Ts>
	using last_t = typename decltype(((std::type_identity<Ts>{}), ...))::type;

	template <typename...> 
	struct ReverseTypeList;

	template <typename...Ts>
	using reverse_type_list_t = typename ReverseTypeList<Ts...>::Type;

	template <typename... t_Types>
	struct TypeList {
		static constexpr size_t k_Size = sizeof...(t_Types);

		template <size_t index>
		using get = typename TypeOfElement<index, t_Types...>::Type;
	};
}

// interfacing with std tuple facilities
namespace std {
	template <typename...Ts>
	struct tuple_size<::bop::util::TypeList<Ts...>>:
		std::integral_constant<size_t, sizeof...(Ts)> 
	{
	};

	template <size_t t_Index, typename...Ts>
	struct tuple_element<t_Index, ::bop::util::TypeList<Ts...>> {
		using type = ::bop::util::type_of_element_t<t_Index, Ts...>;
	};

	template <size_t t_Index, typename...Ts>
	constexpr auto get(::bop::util::TypeList<Ts...>) noexcept {
		return std::type_identity<::bop::util::type_of_element_t<t_Index, Ts...>>{};
	}
}

#include "typelist.inl"