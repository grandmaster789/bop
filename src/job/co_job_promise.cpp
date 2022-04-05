#include "co_job_promise.h"

namespace bop::job {
	CoJobPromiseBase::CoJobPromiseBase(
		std::pmr::memory_resource* resource,
		std::coroutine_handle<>    coro
	) noexcept :
		Job(resource),
		m_Handle(coro)
	{
	}
}