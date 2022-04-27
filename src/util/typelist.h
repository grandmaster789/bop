#pragma once

#include <type_traits>

namespace util {
	// so result_of has been standardized, but there's no standard typelist for args

	template <typename...Ts> 
	struct TypeList {
	};

	template <typename...Ts> struct Head {};
	template <typename...Ts> struct Tail {};
	template <typename...Ts> struct ListSize {
		static constexpr std::size_t k_NumTypes = sizeof...(Ts);
	};

	template <typename T, typename... Ts> struct Head<T, Ts...>           { using type = T; };
	template <typename T, typename... Ts> struct Head<TypeList<T, Ts...>> { using type = T; };
	template <typename T, typename... Ts> struct Tail<T, Ts...>           { using type = TypeList<Ts...>; };
	template <typename T, typename... Ts> struct Tail<TypeList<T, Ts...>> { using type = TypeList<Ts...>; };

	template <typename T, typename... Ts> using head_t = typename Head<T, Ts...>::type;
	template <typename T, typename... Ts> using tail_t = typename Tail<T, Ts...>::type;

	template <typename...Ts> constexpr std::size_t count_types(Ts&&...)           { return sizeof...(Ts); }
	template <typename...Ts> constexpr std::size_t count_types(TypeList<Ts...>&&) { return sizeof...(Ts); }
}