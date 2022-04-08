#pragma once

#include "job_system.h"
#include "job.h"
#include <cassert>

namespace bop::job {
	Job& JobSystem::schedule(
		std::invocable auto&&   fn,
		Job*                    parent,
		std::optional<uint32_t> thread_index
	) {
		Job* work = construct(
			std::forward<decltype(fn)>(fn), 
			parent,
			thread_index
		);

		schedule_work(work);

		return *work;
	}

	Job* JobSystem::construct(
		std::invocable auto&&   fn,
		Job*                    parent,
		std::optional<uint32_t> thread_index
	) noexcept {
		Job* result = create_job(); // construct an empty job first

		result->m_Parent      = parent;       // may be nullptr
		result->m_ThreadIndex = thread_index; // optionally specified		
		result->m_Work        = std::forward<decltype(fn)>(fn);

		return result;
	}
}

namespace bop {
	job::Job& schedule(
		std::invocable auto&&   work,
		job::Job*               parent,
		std::optional<uint32_t> thread_index
	) noexcept {
		// we're adding a single task
		return job::JobSystem()
			.schedule(
				std::forward<decltype(work)>(work),
				parent,
				thread_index
			);
	}
}