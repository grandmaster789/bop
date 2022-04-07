#pragma once

#include "job_system.h"
#include <cassert>

namespace bop::job {
	template <typename T>
	uint32_t JobSystem::schedule(
		T &&                    fn,
		std::optional<uint32_t> thread_index
	) {
		Job* work = construct(
			std::forward<T>(fn), 
			thread_index
		);

		return schedule_work(work);
	}

	template <typename Fn>
	Job* JobSystem::construct(
		Fn&&                    fn,
		std::optional<uint32_t> thread_index
	) noexcept {
		Job* result = create_job(); // construct an empty job first

		result->m_Parent      = nullptr;
		result->m_ThreadIndex = thread_index; // optionally specified

		// forward the lambda/functionpointer/function wrapper/invocable etc to the correct field
		if constexpr (std::is_invocable_v<std::decay<Fn>>) {
			result->m_Work        = std::forward<Fn>(fn);
			result->m_WorkFnPtr   = nullptr;
		}
		else {	
			// see if this is a plain global function pointer
			if constexpr (std::is_pointer_v<std::remove_cvref_t<Fn>>) {
				result->m_WorkFnPtr = fn;
			}
			else {
				// must be some other kind of invocable
				result->m_Work      = fn;
				result->m_WorkFnPtr = nullptr;
			}
		}

		return result;
	}
}

namespace bop {
	template <typename T>
	uint32_t schedule(
		T&&                     work,
		std::optional<uint32_t> thread_index
	) noexcept {
		// we're adding a single task
		return job::JobSystem()
			.schedule(
				std::forward<T>(work), 
				thread_index
			);
	}
}