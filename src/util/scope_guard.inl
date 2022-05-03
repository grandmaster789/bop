#pragma once

#include "scope_guard.h"

namespace bop::util {
	template <typename Fn>
	requires (c_invocable<Fn>&& std::movable<Fn>)
	ScopeGuard<Fn>::ScopeGuard(Fn&& fn) noexcept:
		Fn(std::move(fn))
	{
	}

	template <typename Fn>
	requires (c_invocable<Fn>&& std::movable<Fn>)
	ScopeGuard<Fn>::~ScopeGuard() {
		(*static_cast<Fn*>(this))();
	}
}