#pragma once

#include "job_system.h"
#include <cassert>

namespace bop::job {
	template <typename T>
	uint32_t JobSystem::schedule(
		T &&                    fn,
		Job*                    parent,
		std::optional<uint32_t> thread_index
	) {
		Job* work = construct(std::forward<T>(fn), thread_index);
		work->m_Parent = nullptr;
		return schedule_work(work);
	}

	template <typename Fn>
	Job* JobSystem::construct(
		Fn&&                    fn,
		std::optional<uint32_t> thread_index
	) noexcept {
		Job* result = allocate(); // do allocation first

		// forward the lambda/functionpointer/function wrapper/invocable etc to the correct field
		// 
		// if we can, initialize it using the given thread index
		if constexpr (std::is_invocable_v<std::decay<Fn>>) {
			result->m_Work        = std::forward<Fn>(fn);
			result->m_WorkFnPtr   = nullptr;
			result->m_ThreadIndex = thread_index;
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

	template <typename T>
	void JobSystem::schedule_continuation(T&& fn) noexcept {
		Job* current = get_current_work();

		if (
			!current ||             // no job is currently executing
			!current->is_function() // the executing job is not a coroutine
		)
			return; 

		current->m_Continuation = allocate(std::forward<T>(fn));
	}
}

namespace bop {
	template <typename T>
	uint32_t schedule(
		T&&                     work,
		job::Job*               parent,
		std::optional<uint32_t> thread_index,
		std::optional<uint32_t> num_child_tasks
	) noexcept {
		if constexpr (util::is_pmr_vector<std::decay_t<T>>::value) {
			// we're adding multiple tasks at the same time
			// NOTE do we want to explicitly put this code in a separate function?
			if (!num_child_tasks)
				num_child_tasks = static_cast<uint32_t>(work.size());

			uint32_t result = num_child_tasks;
			
			for (auto&& f : work) {
				schedule(
					std::forward<decltype(f)>(f), 
					parent, 
					num_child_tasks
				);
			
				// only the first job lists the total number of child tasks
				num_child_tasks = 0;
			}

			return result;
		}
		else {
			// we're adding a single task
			return job::JobSystem()
				.schedule(
					std::forward<T>(work), 
					parent, 
					num_child_tasks
				);
		}
	}

	template <typename Fn>
	void schedule_continuation(Fn&& f) noexcept {
		job::JobSystem::schedule_continuation(std::forward<Fn>(f));
	}
}