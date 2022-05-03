#pragma once

#include "concepts.h"

namespace bop::util {
	template <typename Fn>
	requires (c_invocable<Fn> && std::movable<Fn>)
	struct ScopeGuard: Fn {
		explicit ScopeGuard(Fn&& fn) noexcept;
		~ScopeGuard();
	};
}

#include "scope_guard.inl"