#pragma once

namespace bop::util {
	template <typename T, typename... t_Args>
	struct IsImplicitConstructible; // ~> static constexpr bool value;

	template <typename T, typename...Ts>
	constexpr inline bool is_implicit_constructible_v = IsImplicitConstructible<T, Ts...>::value;

	template <typename T>
	constexpr inline bool is_implicit_default_constructible_v = is_implicit_constructible_v<T>;
	template <typename T>
	constexpr inline bool is_implicit_copy_constructible_v = is_implicit_constructible_v<T, const T&>;
	template <typename T>
	constexpr inline bool is_implicit_move_constructible_v = is_implicit_constructible_v<T, T&&>;

	// is instance of
	template <typename T, template <typename...> typename t_Options>
	struct IsInstanceOf; // ~> static constexpr bool value;

	template <typename T, template <typename...> typename t_Options>
	constexpr inline bool is_instance_of_v = IsInstanceOf<T, t_Options...>::value;

	template <typename T, template <typename...> typename t_Options>
	concept c_instance_of = is_instance_of_v<T, t_Options>;

	// A grab-bag of stuff that are kind of utilities for metaprogramming
	//
	//
	struct Null {};

	template <typename T>
	constexpr inline bool is_nulltype_v = std::is_same_v<Null, std::remove_cv_t<T>>;

	                       [[nodiscard]] consteval inline bool always_false(auto&&) noexcept { return false; }
	template <typename...> [[nodiscard]] consteval inline bool always_false()       noexcept { return false; }
	                       [[nodiscard]] consteval inline bool always_true(auto&&)  noexcept { return true; }
	template <typename...> [[nodiscard]] consteval inline bool always_true()        noexcept { return true; }

	// lambda's have unique types, let's make that explicit
	template <auto V = []{} >
	using make_unique_type = decltype(V);

}

#include "traits.inl"