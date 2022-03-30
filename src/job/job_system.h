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

#include "job.h"
#include "job_queue.h"
#include "../util/thread_types.h"
#include "../util/tag.h"
#include "../util/simple_function.h"

namespace bop::job {
	/*
	*	Program wide threadpool for executing Jobs (ie. void()-like invocable things)
	*/
	class JobSystem {
	private:
		static constexpr uint32_t k_RecyclingCapacity = 1 << 10;
		static constexpr bool     k_EnableTracing     = false; // when enabled, a tracing file is produced that can be visualized in Chrome as a flamechart

	public:
		using Clock           = std::chrono::high_resolution_clock;
		using TimePoint       = Clock::time_point;
		using MemoryResource  = std::pmr::memory_resource;
		using JobAllocator    = std::pmr::polymorphic_allocator<Job>;
		using TypeMapping     = std::map<int32_t, std::string>;
		using TagQueues       = std::unordered_map<
			util::Tag, 
			std::unique_ptr<JobQueue<JobBase>>, 
			util::Tag::hash
		>;
		using UniqueMutex     = std::unique_ptr<std::mutex>;
		using TracingData     = std::pmr::vector<std::pmr::vector<JobTrace>>;

		JobSystem(
			util::ThreadCount num_threads     = util::ThreadCount(),
			MemoryResource*   memory_resource = std::pmr::new_delete_resource()
		) noexcept;
		
		JobSystem             (const JobSystem&)     = delete;
		JobSystem& operator = (const JobSystem&)     = delete;
		JobSystem             (JobSystem&&) noexcept = default;
		JobSystem& operator = (JobSystem&&) noexcept = default;

		static bool is_started() noexcept;
		void shutdown() noexcept;
		void wait_for_shutdown() noexcept;

		void on_completed(Job* work) noexcept; // called when all children of a job + itself have completed
		bool child_completed(JobBase* job) noexcept;

		void worker(util::ThreadIndex idx = util::ThreadIndex(0)) noexcept;
		void recycle(Job* work) noexcept;

		static JobBase* get_current_work() noexcept; // yields the thread-local job currently being executed
		
		// this should be the mainly used entrypoint for scheduling work - either
		// some kind of invocable or tag is allowed
		template <typename T>
		requires 
			util::c_functor<T> || 
			std::is_same_v<std::decay_t<T>, util::Tag>
		uint32_t schedule(
			T&&       fn, 
			util::Tag tag        = util::Tag(),
			JobBase*  parent      = m_CurrentJob,
			int32_t   num_children = -1
		);

		util::ThreadIndex get_thread_index() const noexcept;
		util::ThreadCount get_num_threads() const noexcept;
		MemoryResource*   get_memory_resource() const noexcept;

		// tracing/logging features
		void enable_logging();
		void disable_logging();
		void clear_logs();

		TimePoint get_start_time();

		const TypeMapping& get_type_mapping() const;

	private:
		Job* allocate();

		template <typename Fn>
		requires util::c_functor<Fn>
		Job* allocate(Fn&& fn) noexcept;

		// schedules a job, optionally for execution during a tagged phase
		bool schedule_work(
			JobBase*  work, 
			util::Tag tag = util::Tag()
		) noexcept; // returns true if it is scheduled generically and false for a tagged phase

		// schedule all jobs with a given tag
		uint32_t schedule_tag(
			util::Tag tag,
			JobBase*  parent       = l_CurrentJob, // default to the (threadlocal) current job
			int32_t   num_children = -1            // default to all children
		) noexcept;                                // returns the number of jobs scheduled

		template <typename T>
		requires util::c_functor<T>
		void schedule_continuation(T&& fn) noexcept;

		void append_trace(
			const TimePoint&  start,
			const TimePoint&  stop,
			bool              has_completed,
			util::ThreadIndex executing_thread,
			util::ThreadType  job_type,
			util::ThreadID    id
		);

		// pool-related
		static inline std::atomic<uint64_t>    m_InitOnce         = 0;
		static inline MemoryResource*          m_MemoryResource   = nullptr;
		static inline std::vector<std::thread> m_WorkerThreads;	  
		static inline std::atomic<uint32_t>    m_NumThreads       = 0;       // number of threads in the pool (might not need to be atomic)
		static inline std::atomic<bool>        m_Shutdown         = false;   // flag to stop workers
		static inline std::atomic<bool>        m_ShutdownComplete = false;   // when true, all worker threads have stopped

		// queue related
		static inline std::vector<JobQueue<JobBase>> m_GlobalQueues;
		static inline std::vector<JobQueue<JobBase>> m_LocalQueues;
		static inline std::condition_variable        m_WaitCondition;
		static inline std::vector<UniqueMutex>       m_Mutexes;
		static inline TagQueues                      m_TagQueues;

		// per-thread stuff
		static inline thread_local util::ThreadIndex    l_ThreadIndex;
		static inline thread_local JobBase*             l_CurrentJob = nullptr;
		static inline thread_local JobQueue<Job, false> l_RecyclingBin;
		static inline thread_local JobQueue<Job, false> l_GarbageBin;

		// tracing/logging
		static inline bool        m_DoLogging = false;
		static inline TracingData m_Traces;
		static inline TypeMapping m_TypeNames;
		static inline TimePoint   m_StartTime = Clock::now(); // default to program startup time
	};
}

#include "job_system.inl"