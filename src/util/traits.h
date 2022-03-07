#pragma once

namespace bop::util {
	template <typename T, typename... t_Args>
	struct IsImplicitConstructible; // ~> static constexpr bool value;

	// checks typelists/valuelists against one another
	template <typename T, template <typename...> typename t_TypeList>
	struct IsInstanceOf; // ~> static constexpr bool value;

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
}

#include "traits.inl"