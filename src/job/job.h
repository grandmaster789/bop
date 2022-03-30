#pragma once

#include <atomic>
#include <memory_resource>
#include <chrono>

#include "../util/thread_types.h"
#include "../util/simple_function.h"

namespace bop::job {
	class Job;

	struct JobDeallocator {
		virtual void operator()(Job* job) noexcept;
	};

	struct Queueable {
		Queueable* m_Next = nullptr; // singly linked
	};

	class JobBase:
		public Queueable
	{
	public:
		friend class JobSystem;

		JobBase() = default;

		// thread attributes may be adjusted after construction
		util::ThreadIndex m_ThreadIndex;
		util::ThreadType  m_ThreadType;
		util::ThreadID    m_ThreadID;

		virtual bool           resume()          noexcept = 0; // override this to actually do work
		virtual JobDeallocator get_deallocator() noexcept;

		void operator()() noexcept;

		bool is_function() noexcept;

	protected:
		std::atomic<int> m_NumChildren = 0;
		JobBase*         m_Parent      = nullptr;
		bool             m_IsFunction  = false;
	};

	class Job:
		public JobBase
	{
	public:
		friend class JobSystem;

		Job(std::pmr::memory_resource* resource);

		void reset() noexcept;
		bool resume() noexcept override;
		bool deallocate() noexcept;

	protected:
		friend class JobDeallocator; // because this needs to access the memory resource

		std::pmr::memory_resource* m_MemoryResource = nullptr;
		JobBase*                   m_Continuation   = nullptr;
		util::SimpleFunction       m_Work;
		util::void_function_ptr    m_WorkFnPtr;
	};

	// small object to track performance; can be saved and the trace loaded in Chrome about:://tracing
	// TODO FIXME
	struct JobTrace {
		using Clock     = std::chrono::high_resolution_clock;
		using TimePoint = Clock::time_point;

		JobTrace(
			TimePoint         start_time, 
			TimePoint         stop_time,
			bool              task_completed, 
			util::ThreadIndex executing_thread, 
			util::ThreadType  type, 
			util::ThreadID    thread_id
		);

		TimePoint m_StartTime;
		TimePoint m_StopTime;

		bool              m_Completed;
		util::ThreadIndex m_ExecutingThread;
		util::ThreadType  m_ThreadType;
		util::ThreadID    m_ThreadID;
	};
}