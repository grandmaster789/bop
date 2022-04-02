#include "job_system.h"

#include <iostream>
#include <fstream>

namespace bop::job {
	JobSystem::JobSystem(
		std::optional<uint32_t> num_threads,
		MemoryResource*         memory_resource
	) noexcept {
		// make sure that the system is initialized just once
		{
			if (m_InitOnce > 0)
				[[likely]]
				return;

			uint64_t count = m_InitOnce.fetch_add(1);
			if (count > 0)
				return;
		}

		// initalize (static) members
		m_MemoryResource = memory_resource;

		if (!num_threads) 
			m_NumThreads = std::thread::hardware_concurrency(); // default to hardware concurrency count
		else
			m_NumThreads = *num_threads;

		if (m_NumThreads == 0)
			m_NumThreads = 1; // always have at least one worker

		// initialize queue logic for all worker threads
		// sadness - the semantics of vector 
		m_GlobalQueues = std::make_unique<JobQueue[]>(m_NumThreads);
		m_LocalQueues  = std::make_unique<JobQueue[]>(m_NumThreads);
		m_Mutexes      = std::make_unique<std::mutex[]>(m_NumThreads);

		// launch and detach worker threads
		for (uint32_t i = 0; i < m_NumThreads; ++i) {
			m_WorkerThreads.push_back(std::thread(
				&JobSystem::worker, 
				this, 
				i
			));

			m_WorkerThreads[i].detach();
		}		
	}

	bool JobSystem::is_started() noexcept {
		return (l_CurrentJob != nullptr);
	}

	void JobSystem::shutdown() noexcept {
		m_Shutdown.store(true);
	}

	void JobSystem::wait_for_shutdown() noexcept {
		using namespace std::chrono_literals;

		while (!m_ShutdownComplete.load())
			std::this_thread::sleep_for(100ms);
	}

	void JobSystem::on_completed(Job* work) noexcept {
		// if a continuation is present in the work, schedule it
		if (work->m_Continuation) {
			if (work->m_Parent) {
				++work->m_Parent->m_NumChildren;                 // we have more work to do
				work->m_Continuation->m_Parent = work->m_Parent; // propagate the parent from the previous stage
			}

			schedule_work(work->m_Continuation);
		}

		if (work->m_Parent)
			child_completed(work->m_Parent); // let the parent know that a child task has completed

		recycle(work);
	}

	bool JobSystem::child_completed(Job* job) noexcept {
		uint32_t remaining = job->m_NumChildren.fetch_sub(1);

		if (remaining <= 1) { // at some point, there's no more child tasks remaining
			if (job->is_function())
				on_completed(job);
			else
				schedule_work(job); // this is the coro case, which just needs to be rescheduled

			return true; // this job was fully completed
		}

		return false; // more stuff to do
	}

	void JobSystem::worker(uint32_t thread_index) noexcept {
		constexpr uint32_t k_LoopsUntilGC = 1 << 8;

		thread_local static uint32_t l_no_work_counter = 0;

		l_ThreadIndex = thread_index;

		// wait until all threads are started
		{
			using namespace std::chrono_literals;

			static std::atomic<uint32_t> remaining_workers = m_NumThreads.load();

			remaining_workers.fetch_sub(1);

			while (remaining_workers.load() > 0)
				std::this_thread::sleep_for(10ms);
		}

		// set up work-stealing indices
		uint32_t steal_from = (thread_index + 1) % m_NumThreads.load();

		std::unique_lock lock(m_Mutexes[l_ThreadIndex]);

		// main worker loop -- do work while we're shutting down
		while (!m_Shutdown) {
			// prioritize local queue over the global one (should have less contention)
			l_CurrentJob = m_LocalQueues[l_ThreadIndex].pop();

			if (!l_CurrentJob)
				l_CurrentJob = m_GlobalQueues[l_ThreadIndex].pop();

			// if we still don't have any work yet try to steal it from another global queue
			uint32_t numStealAttempts = m_NumThreads - 1;
			while (!l_CurrentJob && (--numStealAttempts > 0)) {
				if (++steal_from >= m_NumThreads)
					steal_from = 0;

				l_CurrentJob = m_GlobalQueues[steal_from].pop();
			}

			// if we have a job, execute it
			if (l_CurrentJob) {
				// before doing the work, remember if this was a regular function or a coroutine
				// (in the coro case the job may destroy itself)
				bool is_function = l_CurrentJob->is_function(); 

				(*l_CurrentJob)(); // do the actual work
				
				if (is_function)
					child_completed(l_CurrentJob);

				l_no_work_counter = 0;
			}
			// if we still don't have work for a long time we may try reclaiming some memory
			else if (++l_no_work_counter > k_LoopsUntilGC) [[unlikely]] {
				using namespace std::chrono_literals;

				l_GarbageBin.clear();
				m_WaitCondition.wait_for(lock, 100ms);

				l_no_work_counter /= 2; // it's not like this worker is hot, so don't reset the counter entirely
			}
		}

		// we're outside of the execution loop; shutdown was triggered so do cleanup this workers' resources
		m_GlobalQueues[l_ThreadIndex].clear();
		m_LocalQueues[l_ThreadIndex].clear();
		l_RecyclingBin.clear();
		l_GarbageBin.clear();

		// the last worker may save the trace report
		uint32_t active_workers = m_NumThreads.fetch_sub(1);
		if (active_workers == 1)
			m_ShutdownComplete = true;
	}

	void JobSystem::recycle(Job* work) noexcept {
		if (l_RecyclingBin.size() <= k_RecyclingCapacity)
			l_RecyclingBin.push(work); // tag for re-use
		else
			l_GarbageBin.push(work); // tag as garbage
	}

	Job* JobSystem::get_current_work() noexcept {
		return l_CurrentJob;
	}
	
	uint32_t JobSystem::get_thread_index() const noexcept {
		return l_ThreadIndex;
	}

	uint32_t JobSystem::get_num_threads() const noexcept {
		return m_NumThreads;
	}

	JobSystem::MemoryResource* JobSystem::get_memory_resource() const noexcept {
		return m_MemoryResource;
	}

	Job* JobSystem::allocate() {
		// first see if we have anything we can recycle
		Job* result = l_RecyclingBin.pop();

		if (!result) {
			// do an actual allocation in that case
			// NOTE maybe make the allocator a member?
			//      that would make cleanup more difficult though...
			JobAllocator allocator(m_MemoryResource);

			result = allocator.new_object<Job>(m_MemoryResource);

			if (!result) {
				std::cerr << "Failed to create a job\n";
				std::terminate();
			}
		}
		else
			result->reset();

		return result;
	}

	bool JobSystem::schedule_work(Job* work) noexcept {
		static thread_local uint32_t tidx(0); // simplest possible load-balancing

		// if no specific execution thread was set, plonk it any global queue
		if (
			(!work->m_ThreadIndex) ||
			(*work->m_ThreadIndex >= m_NumThreads)
		) {
			++tidx;
			if (tidx >= m_NumThreads)
				tidx = 0;

			m_GlobalQueues[tidx].push(work);
			m_WaitCondition.notify_one(); // wake up a thread (if any was waiting)

			return true;
		}

		// if a specific execution thread was set, plonk it in the appropriate local queue
		m_LocalQueues[l_ThreadIndex].push(work);
		m_WaitCondition.notify_one();

		return true;
	}
}

namespace bop {
	job::Job* current_work() {
		return job::JobSystem::get_current_work();
	}

	void shutdown() {
		job::JobSystem::shutdown();
	}

	void wait_for_shutdown() {
		job::JobSystem::wait_for_shutdown();
	}
}