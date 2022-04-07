#include "co_job_promise.h"

namespace bop::job {
	CoJobPromiseBase::CoJobPromiseBase(std::coroutine_handle<> coro) noexcept:
		m_Handle(coro)
	{
	}
}