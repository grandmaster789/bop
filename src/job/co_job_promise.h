#pragma once

#include "job.h"
#include <coroutine>

namespace bop::job {
	struct CoJob;

	struct CoJobPromise {
		CoJob get_return_object();
		std::suspend_always initial_suspend() noexcept;
		std::suspend_never final_suspend() noexcept;
		void return_void();
		void unhandled_exception();
	};
}