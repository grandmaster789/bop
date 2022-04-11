#pragma once

#include <coroutine>

namespace bop::job {
	struct CoJobPromise;

	struct CoJob:
		std::coroutine_handle<CoJobPromise>
	{
		using promise_type = struct CoJobPromise;
	};
}