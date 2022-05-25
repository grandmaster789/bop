#pragma once

#include <atomic>
#include <variant>
#include <optional>
#include <exception>
#include <functional>
#include <type_traits>
#include <coroutine>

#include <iostream>

#include "../util/concepts.h"
#include "../util/scope_guard.h"
#include "../util/typelist.h"
#include "../util/function.h"

namespace bop::task {
	class Task;

	// [NOTE] is a base class even required?
	//        we sort of need one to account for coroutines, but the links may end up needing dynamic type information in that case...
	//        the other need for base classes is when we want to store result values inside of the task object itself;
	//        which may be problematic. Memory allocation (PMR-based) is done in the scheduler, but might be factored out?
	class TaskBase {
	public:
		constexpr TaskBase() noexcept = default;
		virtual ~TaskBase() = default;

		inline void reset();
		inline void wait();

		inline void set_thread_index(uint32_t thread_index) {
			m_ThreadIndex = thread_index;
		}

	protected:
		friend class TaskQueue;

		std::atomic<uint32_t>   m_NumDependencies = 1;            // number of child tasks this one is waiting for; on 1 this task may be executed
		Task*                   m_Parent          = nullptr;      // backpointer to the task that initiated this one
		Task*                   m_NextLink        = nullptr;      // intrusive pointer for the next task to be executed after this one
		std::optional<uint32_t> m_ThreadIndex     = std::nullopt; // if set, forces execution on a particular thread
	};
	
	class Task:
		public TaskBase
	{
	public:
		using VoidFunctionPtr = void(*)();

		Task() = default;

		void operator()() noexcept {
			if (m_FunctionPtr) [[unlikely]]
				m_FunctionPtr();
			else
				m_Work();
		}

	private:
		TaskBase*              m_Continuation = nullptr;
		VoidFunctionPtr        m_FunctionPtr  = nullptr; // NOTE may be consolidated via std::variant
		util::Function<void()> m_Work;
	};
}

#include "task.inl"