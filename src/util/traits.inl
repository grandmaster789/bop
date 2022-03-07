#pragma once

#include "traits.h"
#include <type_traits>

namespace bop::util {
	// IsImplicitContructible
	template <typename T, typename... t_Args>
	struct IsImplicitConstructible {
	private:
		template <typename U> static void check(U&&);

		template <typename... Xs>
		static consteval auto fn(int) -> decltype(check<T>({ std::declval<Xs>()... }), bool{}) {
			return true;
		}

		template <typename...>
		static consteval bool fn(...) {
			return false;
		}

	public:
		static constexpr bool value = fn<t_Args...>(0);
	};

	// IsInstanceOf
	template <typename T, template <typename...> typename t_TypeList>
	struct IsInstanceOf {
	private:
		template <typename>
		struct Check : std::false_type {};

		template <typename...t_Args>
		struct Check<t_TypeList<t_Args...>> : std::true_type {};

	public:
		static constexpr bool value = Check<std::remove_cvref_t<T>>::value;
	};
}