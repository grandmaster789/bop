#pragma once

#include <atomic>
#include <variant>
#include <optional>
#include <exception>
#include <functional>
#include <type_traits>

namespace bop::task {
	/*
	* Basically a 'future' style pattern; the task should have a promise state
	* as well as some kind of function/job/task to fill the promise.
	* 
	* I'd like to make this so that we can have any number of arguments for the
	* function/job/task
	* 
	* We're going with a base class so that we can uniformly use a linked list
	* for deferred execution.
	* 
	* First pass will not be based on coroutine support yet, nor continuations
	*/
	class TaskBase {
	public:
		TaskBase() noexcept = default;

		inline void reset();
		inline void wait();

	protected:
		std::atomic<uint32_t>   m_NumDependencies = 1;
		TaskBase*               m_Parent          = nullptr;
		TaskBase*               m_NextLink        = nullptr;
		std::optional<uint32_t> m_ThreadIndex     = std::nullopt;
	};
}

#include "task.inl"