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

#include "job.h"
#include "job_queue.h"
#include "../util/traits.h"

namespace bop::job {
	/*
	*	Program wide threadpool for executing Jobs (ie. void()-like invocable things)
	*/
	class JobSystem {
	private:
		static constexpr uint32_t k_RecyclingCapacity = 1 << 10;

	public:
		using MemoryResource  = std::pmr::memory_resource;
		using JobAllocator    = std::pmr::polymorphic_allocator<Job>;
		using UniqueMutex     = std::unique_ptr<std::mutex>;
		using UniqueJobQueue  = std::unique_ptr<JobQueue>;

		JobSystem(
			std::optional<uint32_t> num_threads     = std::nullopt,                   // by default this will use the hardware concurrency
			MemoryResource*         memory_resource = std::pmr::new_delete_resource()
		) noexcept;
		
		JobSystem             (const JobSystem&)     = delete;
		JobSystem& operator = (const JobSystem&)     = delete;
		JobSystem             (JobSystem&&) noexcept = default;
		JobSystem& operator = (JobSystem&&) noexcept = default;

		static bool is_started() noexcept;
		static void shutdown() noexcept;
		static void wait_for_shutdown() noexcept;

		void on_completed(Job* work) noexcept; // called when all children of a job + itself have completed
		bool child_completed(Job* job) noexcept;

		void worker(uint32_t thread_index) noexcept;
		void recycle(Job* work) noexcept;

		static Job* get_current_work() noexcept; // yields the thread-local job currently being executed
		
		// this should be the mainly used entrypoint for scheduling work - either
		// some kind of invocable or tag is allowed
		template <typename T>
		uint32_t schedule(
			T&&                     fn, 
			Job*                    parent       = m_CurrentJob,
			std::optional<uint32_t> thread_index = std::nullopt
		);

		uint32_t        get_thread_index() const noexcept;
		uint32_t        get_num_threads() const noexcept;
		MemoryResource* get_memory_resource() const noexcept;

	private:
		Job* allocate();

		template <typename Fn>
		Job* construct(
			Fn&&                    fn, 
			std::optional<uint32_t> thread_index = std::nullopt
		) noexcept;

		bool schedule_work(Job* work) noexcept; // returns true if it is scheduled generically and false for a tagged phase

		template <typename T>
		void schedule_continuation(T&& fn) noexcept;

		// pool-related
		static inline std::atomic<uint64_t>    m_InitOnce         = 0;
		static inline MemoryResource*          m_MemoryResource   = nullptr;
		static inline std::vector<std::thread> m_WorkerThreads;	  
		static inline std::atomic<uint32_t>    m_NumThreads       = 0;       // number of threads in the pool
		static inline std::atomic<bool>        m_Shutdown         = false;   // flag to stop workers
		static inline std::atomic<bool>        m_ShutdownComplete = false;   // when true, all worker threads have stopped

		// queue related
		// (the unique_ptr to the queue is to make the queues themselves memory-stable)
		static inline std::vector<UniqueJobQueue> m_GlobalQueues;
		static inline std::vector<UniqueJobQueue> m_LocalQueues;
		static inline std::condition_variable     m_WaitCondition;
		static inline std::vector<UniqueMutex>    m_Mutexes;

		// per-thread stuff
		static inline thread_local uint32_t              l_ThreadIndex;
		static inline thread_local Job*                  l_CurrentJob  = nullptr;
		static inline thread_local JobQueueNonThreadsafe l_RecyclingBin;
		static inline thread_local JobQueueNonThreadsafe l_GarbageBin;
	};
}

namespace bop {
	job::Job* current_work();

	template <typename T>
	uint32_t schedule(
		T&&                     work,
		job::Job*               parent          = current_work(),
		std::optional<uint32_t> thread_index    = std::nullopt,
		std::optional<uint32_t> num_child_tasks = std::nullopt
	) noexcept;

	template <typename Fn>
	void continuation(Fn&& f) noexcept;

	void shutdown();
	void wait_for_shutdown();
}

#include "job_system.inl"