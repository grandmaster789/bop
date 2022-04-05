#pragma once

#include "job.h"
#include <coroutine>

namespace bop::job {
	class CoJobPromiseBase :
		public Job // so this can be scheduled
	{
	public:
		explicit CoJobPromiseBase(
			std::pmr::memory_resource* resource,
			std::coroutine_handle<>    coro
		) noexcept;

	protected:
		std::coroutine_handle<> m_Handle;
	};
}