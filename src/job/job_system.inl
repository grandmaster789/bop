#pragma once

#include "job_system.h"
#include <cassert>

namespace bop::job {
	template <typename T>
	requires
		util::c_functor<T> ||
		std::is_same_v<std::decay_t<T>, util::Tag>
	uint32_t JobSystem::schedule(
		T && fn,
		util::Tag tag,
		JobBase * parent,
		int32_t   num_children
	) {
		if constexpr (std::is_same_v<std::decay<T>, util::Tag>) {
			return schedule_tag(fn, tag, parent, num_children);
		}

		Job* work = allocate(std::forward<T>(fn));
		work->m_Parent = nullptr;

		if (tag.m_Value < 0) {
			work->m_Parent = parent;

			if (parent) {
				if (num_children < 0)
					num_children = 1; // this is the first job in its tree

				// add the child task to the parents' count
				parent->m_NumChildren.fetch_add(static_cast<int>(num_children));
			}
		}

		return schedule_work(work, tag);
	}

	template <typename Fn>
	requires util::c_functor<Fn>
	Job* JobSystem::allocate(Fn&& fn) noexcept {
		Job* result = allocate(); // use basic allocation first

		// forward the lambda/functionpointer/function wrapper/invocable etc to the correct field
		// 
		// if we can, initialize it using SimpleFunction members
		if constexpr (std::is_same_v<std::decay<Fn>, util::SimpleFunction>) {
			result->m_Work        = fn.get_function();
			result->m_WorkFnPtr   = nullptr;
			result->m_ThreadIndex = fn.m_ThreadIndex;
			result->m_ThreadType  = fn.m_ThreadType;
			result->m_ThreadID    = fn.m_Thread_ID;
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
	requires util::c_functor<T>
	void JobSystem::schedule_continuation(T&& fn) noexcept {
		JobBase* current = get_current_work();

		if (
			!current ||             // no job is currently executing
			!current->is_function() // the executing job is not a coroutine
		)
			return; 

		static_cast<Job*>(current)->m_Continuation = allocate(std::forward<T>(fn));
	}
}