#include "co_job_promise.h"
#include "co_job.h"

namespace bop::job {
	CoJob CoJobPromise::get_return_object() {
		return { CoJob::from_promise(*this) };
	}

	std::suspend_always CoJobPromise::initial_suspend() noexcept {
		return {};
	}

	std::suspend_never CoJobPromise::final_suspend() noexcept {
		return {};
	}

	void CoJobPromise::return_void() {
	}

	void CoJobPromise::unhandled_exception() {
	}
}