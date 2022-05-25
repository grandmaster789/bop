#pragma once

#include "concepts.h"

namespace bop::util {
	// inheritance-based -- i.e. this should be derived from a lambda
	//
	template <typename Fn>
	requires (c_invocable<Fn> && std::movable<Fn>)
	struct ScopeGuard: Fn {
		explicit ScopeGuard(Fn&& fn) noexcept;
		~ScopeGuard();
	};
}

#include "scope_guard.inl"