#pragma once

#include "typelist.h"
#include "valuelist.h"

namespace bop::util {
	namespace detail {
		// Type Of Element Helper
		template <
			size_t,    // index
			size_t,    // cursor
			typename... // type list
		>
		struct TOEHelper;

		template <
			size_t t_Index,
			typename T, 
			typename... t_Types
		>
		struct TOEHelper<t_Index, t_Index, T, t_Types...> {
			using Type = T;
		};

		template <
			size_t t_Index,
			size_t t_Cursor,
			typename t_Head, 
			typename... t_Tail
		>
		struct TOEHelper<t_Index, t_Cursor, t_Head, t_Tail...> {
			using Type = TOEHelper<t_Index, t_Cursor + 1, t_Tail...>::Type;
		};

		// reverse type list helper
		// (ReverseTypeList creates an inverted list of indices for the types)
		template <typename...>
		struct ReverseTypeListHelper;

		template <
			typename... t_Types,
			size_t... t_Indices
		>
			struct ReverseTypeListHelper<ValueList<size_t, t_Indices...>, t_Types...> {
			using InternalTypelist = typename TypeList<t_Types...>;
			using Type = TypeList<typename InternalTypelist::template get<t_Indices>...>;
		};
	}

	template <
		size_t t_Index,
		typename... t_Types
	>
	struct TypeOfElement {
		static_assert(t_Index < sizeof...(t_Types), "Index out of bounds");
		using Type = typename detail::TOEHelper<t_Index, 0, t_Types...>::Type;
	};

	// Extract type list
	template <
		template <typename...> typename t_Template, 
		typename... t_Types
	>
	struct ExtractTypeList<t_Template<t_Types...>> {
		using Type = TypeList<t_Types...>;
	};

	// Merge type lists
	template <
		template <typename...> typename t_TypeListA,
		template <typename...> typename t_TypeListB,
		typename... t_ArgsA,
		typename... t_ArgsB
	>
	struct MergeTypeLists<
		t_TypeListA<t_ArgsA...>, 
		t_TypeListB<t_ArgsB...>
	> {
		using Type = TypeList<t_ArgsA..., t_ArgsB...>;
	};

	template <
		template <typename...> typename t_TypeListA,
		template <typename...> typename t_TypeListB,
		typename... t_ArgsA,
		typename... t_ArgsB,
		typename... t_AdditionalTypes
	> 
	struct MergeTypeLists<
		t_TypeListA<t_ArgsA...>,
		t_TypeListB<t_ArgsB...>,
		t_AdditionalTypes...
	> {
		using Type = typename MergeTypeLists<
			TypeList<t_ArgsA..., t_ArgsB...>, 
			t_AdditionalTypes...
		>::Type;
	};

	// insert type list
	template <
		template <typename...> typename t_Template, 
		typename... t_Types
	> 
	struct InsertTypeList<t_Template, t_Types...> {
		using Type = t_Template<t_Types...>;
	};

	// remove first type
	template <
		template <typename...> typename t_Template,
		typename t_Head, 
		typename... t_Tail
	> struct RemoveFirstType<t_Template<t_Head, t_Tail...>> {
		using Type = typename t_Template<t_Tail...>;
	};

	// reverse type list
	template <typename... t_Values>
	struct ReverseTypeList {
		// 
		using Type = typename detail::ReverseTypeListHelper<
			ValueSequence<
				int,							// value type
				sizeof...(t_Values),			// number of elements
				(int)(sizeof...(t_Values)) - 1, // starting value
				0,								// cursor
				-1								// step size
			>, 
			t_Values...
		>::Type;
	};

	template <typename... t_Types>
	struct ReverseTypeList<TypeList<t_Types...>> {
		using Type = typename ReverseTypeList<t_Types...>::type;
	};
}