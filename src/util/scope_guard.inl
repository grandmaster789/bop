#pragma once

#include "scope_guard.h"

namespace bop::util {
	template <typename Fn>
	ScopeGuard<Fn>::ScopeGuard(Fn&& fn) noexcept:
		Fn(std::move(fn))
	{
	}

	template <typename Fn>
	ScopeGuard<Fn>::~ScopeGuard() {
		Fn::operator()();
	}
}