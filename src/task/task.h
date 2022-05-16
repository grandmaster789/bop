#pragma once

#include <atomic>
#include <variant>
#include <optional>
#include <exception>
#include <functional>
#include <type_traits>

#include "../util/concepts.h"

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
		constexpr TaskBase() noexcept = default;
		virtual ~TaskBase() = default;

		inline void reset();
		inline void wait();

	protected:
		std::atomic<uint32_t>   m_NumDependencies = 1;            // number of child tasks this one is waiting for; on 1 this task may be executed
		TaskBase*               m_Parent          = nullptr;      // backpointer to the task that initiated this one
		TaskBase*               m_NextLink        = nullptr;      // intrusive pointer for the next task to be executed after this one
		std::optional<uint32_t> m_ThreadIndex     = std::nullopt; // if set, forces execution on a particular thread
	};

	// use a template to allow inheriting from a lambda
	template <util::c_invocable Fn>
	class Task: 
		public TaskBase,
		public Fn
	{
	public:
		constexpr Task(Fn&& fn) noexcept; // this constructor should allow for automatic derivation of types

		void operator()();

		bool is_done() const noexcept;

	private:
		bool m_Done = false;
	};
}

#include "task.inl"