#pragma once

#include "concepts.h"

namespace bop::util {
	template <typename Fn>
	struct ScopeGuard: Fn {
		explicit ScopeGuard(Fn&& fn) noexcept;
		~ScopeGuard();
	};

	// usage:
	//    auto guard = util::ScopeGuard([this] { m_Done = true; });
}

#include "scope_guard.inl"