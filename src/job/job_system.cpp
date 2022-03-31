#include "job_system.h"

#include <iostream>
#include <fstream>

namespace bop::job {
	JobSystem::JobSystem(
		util::ThreadCount num_threads,
		MemoryResource*   memory_resource
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
		
		if (num_threads.m_Count < 0) // NOTE m_NumThreads is unsigned, so check the signed source
			m_NumThreads = std::thread::hardware_concurrency();
		else
			m_NumThreads = static_cast<uint32_t>(num_threads.m_Count);

		if (m_NumThreads == 0)
			m_NumThreads = 1; // always have at least one worker

		// initialize queue logic for all worker threads
		for (int32_t i = 0; i < m_NumThreads; ++i) {
			m_GlobalQueues.push_back(std::make_unique<JobQueue>());
			m_LocalQueues.push_back(std::make_unique<JobQueue>());
			m_Mutexes.push_back(std::make_unique<std::mutex>());
		}

		// launch and detach worker threads
		for (uint32_t i = 0; i < m_NumThreads; ++i) {
			m_WorkerThreads.push_back(std::thread(
				&JobSystem::worker, 
				this, 
				util::ThreadIndex{ static_cast<int>(i) }
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
				on_completed(static_cast<Job*>(job));
			else
				schedule_work(job); // this is the coro case, which just needs to be rescheduled

			return true; // this job was fully completed
		}

		return false; // more stuff to do
	}

	void JobSystem::worker(util::ThreadIndex idx) noexcept {
		constexpr uint32_t k_LoopsUntilGC = 1 << 8;

		thread_local static uint32_t l_no_work_counter = 0;

		l_ThreadIndex = idx;

		// wait until all threads are started
		{
			using namespace std::chrono_literals;

			static std::atomic<uint32_t> remaining_workers = m_NumThreads.load();

			remaining_workers.fetch_sub(1);

			while (remaining_workers.load() > 0)
				std::this_thread::sleep_for(10ms);
		}

		// set up work-stealing indices
		uint32_t steal_from = static_cast<uint32_t>(idx.m_Index + 1) % m_NumThreads.load();

		std::unique_lock lock(*m_Mutexes[l_ThreadIndex.m_Index]);

		// do work while we're not done
		while (!m_Shutdown) {
			// prioritize local queue over the global one (should have less contention)
			l_CurrentJob = m_LocalQueues[l_ThreadIndex.m_Index]->pop();

			if (!l_CurrentJob)
				l_CurrentJob = m_GlobalQueues[l_ThreadIndex.m_Index]->pop();

			// if we still don't have any work yet try to steal it from another global queue
			uint32_t numStealAttempts = m_NumThreads - 1;
			while (!l_CurrentJob && (--numStealAttempts > 0)) {
				if (++steal_from >= m_NumThreads)
					steal_from = 0;

				l_CurrentJob = m_GlobalQueues[steal_from]->pop();
			}

			// if we have a job, execute it
			if (l_CurrentJob) {
				// before doing the work, remember if this was a regular function or a coroutine
				// (in the coro case the job may destroy itself)
				bool is_function = l_CurrentJob->is_function(); 

				(*l_CurrentJob)(); // do the actual work

				if (is_function)
					child_completed(static_cast<Job*>(l_CurrentJob));

				l_no_work_counter = 0;
			}
			// if we still don't have work for a long time we may try reclaiming some memory
			else if (++l_no_work_counter > k_LoopsUntilGC) [[unlikely]] {
				using namespace std::chrono_literals;

				l_GarbageBin.clear();
				m_WaitCondition.wait_for(lock, 100ms);

				l_no_work_counter /= 2;
			}
		}

		// we're outside of the execution loop; shutdown was triggered so do cleanup this workers' resources
		m_GlobalQueues[l_ThreadIndex.m_Index]->clear();
		m_LocalQueues[l_ThreadIndex.m_Index]->clear();
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
	
	util::ThreadIndex JobSystem::get_thread_index() const noexcept {
		return l_ThreadIndex;
	}

	util::ThreadCount JobSystem::get_num_threads() const noexcept {
		return util::ThreadCount(m_NumThreads.load());
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
		static thread_local util::ThreadIndex tidx(0);

		// if no specific execution thread was set, plonk it any global queue
		if (
			(work->m_ThreadIndex.m_Index < 0) ||
			(work->m_ThreadIndex.m_Index >= static_cast<int>(m_NumThreads))
		) {
			++tidx.m_Index;
			if (tidx.m_Index >= m_NumThreads)
				tidx.m_Index = 0;

			m_GlobalQueues[tidx.m_Index]->push(work);
			m_WaitCondition.notify_one(); // wake up a thread (if any was waiting)

			return true;
		}

		// if a specific execution thread was set, plonk it in the appropriate local queue
		m_LocalQueues[work->m_ThreadIndex.m_Index]->push(work);
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