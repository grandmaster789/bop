#pragma once

#include "concepts.h"

namespace bop::util {
	// inheritance-based -- i.e. this should be derived from a lambda
	//
	template <typename Fn>
	struct ScopeGuard: Fn {
		explicit ScopeGuard(Fn&& fn) noexcept;
		~ScopeGuard();
	};

	// usage:
	//    auto guard = util::ScopeGuard([this] { m_Done = true; });
}

#include "scope_guard.inl"