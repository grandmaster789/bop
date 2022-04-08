#pragma once

#include <atomic>
#include <cstdint>
#include <chrono>
#include <map>
#include <memory>
#include <memory_resource>
#include <thread>
#include <unordered_map>
#include <vector>
#include <optional>

#include "job_queue.h"
#include "job_trace.h"
#include "../util/traits.h"

namespace bop::job {
	class Job;

	/*
	*	Program wide threadpool for executing Jobs (ie. void()-like invocable things)
	*   The system owns the actual jobs, and takes care of memory management as needed
	*/
	class JobSystem {
	private:
		static constexpr uint32_t k_RecyclingCapacity = 1 << 10;
		static constexpr bool     k_EnableProfiling   = true; // when true, a tracelog.json is generated at shutdown that can be viewed in chrome about://tracing
		
	public:
		friend class Job;

		using MemoryResource = std::pmr::memory_resource;
		using JobAllocator   = std::pmr::polymorphic_allocator<Job>;
		using TraceLog       = std::pmr::vector<JobTrace>;
		using JobQueueArray  = std::unique_ptr<JobQueue[]>;
		using MutexArray     = std::unique_ptr<std::mutex[]>;
		using Clock          = std::chrono::high_resolution_clock;
		using Timepoint      = Clock::time_point;

		// uses the PMR composable memory allocation backend
		JobSystem(
			std::optional<uint32_t> num_threads     = std::nullopt,                   // by default this will use the hardware concurrency
			MemoryResource*         memory_resource = std::pmr::new_delete_resource()
		) noexcept;

		JobSystem             (const JobSystem&)     = delete;
		JobSystem& operator = (const JobSystem&)     = delete;
		JobSystem             (JobSystem&&) noexcept = default;
		JobSystem& operator = (JobSystem&&) noexcept = default;

		static void shutdown() noexcept;          // this can be scheduled as a job
		static void wait_for_shutdown() noexcept; // blocks until all workers have stopped

		void worker(uint32_t thread_index) noexcept; // executed on a worker thread
		
		// this should be the mainly used entrypoint for scheduling work - either
		// some kind of invocable or tag is allowed
		inline Job& schedule(
			std::invocable auto&&   fn, 
			Job*                    parent       = nullptr,      // if set, indicates which job waits for this one to complete
			std::optional<uint32_t> thread_index = std::nullopt
		);

		uint32_t        get_thread_index()    const noexcept; // thread-local
		uint32_t        get_num_threads()     const noexcept; // same everywhere
		MemoryResource* get_memory_resource() const noexcept; // exposing this allows coroutines to make use of it to allocate their stackframes

	private:
		Job* create_job();
		void deallocate_job_queue(JobQueue& jq);
		void deallocate_job_queue(JobQueueNonThreadsafe& jq);
		
		inline Job* construct(
			std::invocable auto&&   fn, 
			Job*                    parent,
			std::optional<uint32_t> thread_index
		) noexcept;

		bool schedule_work(Job* work) noexcept; // returns true if it is scheduled generically and false for a tagged phase

		bool job_completed(Job* job) noexcept;
		void recycle(Job* work) noexcept;

		void store_trace(
			const Timepoint& job_start,
			const Timepoint& job_end,
			uint32_t         executing_thread_index
		);
		void save_tracelog();
		void clear_tracelog();

		// pool-related
		static inline std::atomic<uint64_t>    m_InitOnce         = 0;
		static inline MemoryResource*          m_MemoryResource   = nullptr;
		static inline std::vector<std::thread> m_WorkerThreads;	  
		static inline std::atomic<uint32_t>    m_NumThreads       = 0;       // number of threads in the pool
		static inline std::atomic<bool>        m_Shutdown         = false;   // flag to stop workers
		static inline std::atomic<bool>        m_ShutdownComplete = false;   // when true, all worker threads have stopped

		// queue related (these are accessible from all running workers)
		static inline JobQueueArray            m_GlobalQueues;
		static inline JobQueueArray            m_LocalQueues;
		static inline std::condition_variable  m_WaitCondition;
		static inline MutexArray               m_Mutexes;

		// profiling/tracing/logging
		static inline Timepoint                  m_ApplicationStart;
		static inline std::pmr::vector<TraceLog> m_Traces;
		static inline bool                       m_DoLogging = false;

		// per-thread stuff
		static inline thread_local uint32_t              l_ThreadIndex;
		static inline thread_local Job*                  l_CurrentJob  = nullptr;
		static inline thread_local JobQueueNonThreadsafe l_RecyclingBin;
		static inline thread_local JobQueueNonThreadsafe l_GarbageBin;
	};
}

// convenience functions that appropriately forward to the job system
namespace bop {
	inline job::Job& schedule(
		std::invocable auto&&   work,
		job::Job*               parent       = nullptr, // indicates which job is waiting for this one
		std::optional<uint32_t> thread_index = std::nullopt
	) noexcept; // returns the number of jobs scheduled

	void shutdown();
	void wait_for_shutdown();
}

#include "job_system.inl"