#include "job_system.h"
#include "job.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <format>

namespace bop::job {
	JobSystem::JobSystem(
		std::optional<uint32_t> num_threads,
		MemoryResource*         memory_resource
	) noexcept {
		// make sure that the system is initialized just once
		// (we're initializing static variables via a non-static object construction)
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

		m_ApplicationStart = Clock::now();

		// initialize queue logic for all worker threads
		// sadness - the semantics of vector 
		m_GlobalQueues = std::make_unique<JobQueue[]>(m_NumThreads);
		m_LocalQueues  = std::make_unique<JobQueue[]>(m_NumThreads);
		m_Mutexes      = std::make_unique<std::mutex[]>(m_NumThreads);

		m_Traces.resize(m_NumThreads, TraceLog(memory_resource));

		// launch and detach worker threads
		for (uint32_t i = 0; i < m_NumThreads; ++i) {
			m_WorkerThreads.push_back(std::thread(
				&JobSystem::worker, 
				this, 
				i // thread index
			));

			m_WorkerThreads[i].detach();
		}		
	}

	void JobSystem::shutdown() noexcept {
		m_Shutdown.store(true);
	}

	void JobSystem::wait_for_shutdown() noexcept {
		using namespace std::chrono_literals;

		while (!m_ShutdownComplete.load())
			std::this_thread::sleep_for(100ms);
	}

	bool JobSystem::job_completed(Job* job) noexcept {
		// if we have a continuation, schedule it
		if (job->m_Continuation) {
			if (job->m_Parent) {
				job->m_Parent->m_NumChildren++;                // indicate that we have one more thing to wait on
				job->m_Continuation->m_Parent = job->m_Parent; // forward the job parent to the continuation
			}

			schedule_work(job->m_Continuation);
		}

		uint32_t remaining = job->m_NumChildren.fetch_sub(1);

		// at some point, there's no more task dependencies remaining
		if (remaining <= 1) { 
			if (job->m_Parent)
				job_completed(job->m_Parent);

			recycle(job);
			
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
				Timepoint job_start;

				if constexpr (k_EnableProfiling) {
					job_start = Clock::now();
				}

				// before doing the work, remember if this was a regular function or a coroutine
				// (in the coro case the job may destroy itself)
				(*l_CurrentJob)(); // do the actual work
				
				if constexpr (k_EnableProfiling) {
					store_trace(
						job_start,
						Clock::now(),
						l_ThreadIndex
					);
				}

				job_completed(l_CurrentJob);

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
		deallocate_job_queue(m_GlobalQueues[l_ThreadIndex]);
		deallocate_job_queue(m_LocalQueues[l_ThreadIndex]);
		deallocate_job_queue(l_RecyclingBin);
		deallocate_job_queue(l_GarbageBin);
		
		// the last worker may save the trace report
		uint32_t active_workers = m_NumThreads.fetch_sub(1);
		if (active_workers == 1) {
			if constexpr (k_EnableProfiling) {
				save_tracelog();
			}

			m_ShutdownComplete = true;
		}
	}

	void JobSystem::recycle(Job* work) noexcept {
		if (l_RecyclingBin.size() <= k_RecyclingCapacity)
			l_RecyclingBin.push(work); // tag for re-use
		else
			l_GarbageBin.push(work); // tag as garbage
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

	Job* JobSystem::create_job() {
		// first see if we have anything we can recycle
		Job* result = l_RecyclingBin.pop();

		if (!result) {
			// do an actual allocation in that case
			JobAllocator allocator(m_MemoryResource);

			result = allocator.allocate(1);

			if (!result) {
				std::cerr << "Failed to allocate a job\n";
				std::terminate();
			}

			allocator.construct(result);
		}
		else {
			result->reset(); // re-use a previously allocated job
		}

		return result;
	}

	void JobSystem::deallocate_job_queue(JobQueue& jq) {
		JobAllocator allocator(m_MemoryResource);

		for (auto* job = jq.pop(); job; job = jq.pop()) {
			//allocator.destroy(job); // jobs don't have destructors, no need
			allocator.deallocate(job, 1);
		}

		jq.clear();
	}

	void JobSystem::deallocate_job_queue(JobQueueNonThreadsafe& jq) {
		JobAllocator allocator(m_MemoryResource);

		for (auto* job = jq.pop(); job; job = jq.pop()) {
			//allocator.destroy(job); // jobs don't have destructors, no need
			allocator.deallocate(job, 1);
		}

		jq.clear();
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

	void JobSystem::store_trace(
		const Timepoint& job_start,
		const Timepoint& job_current,
		uint32_t         executing_thread_index
	) {
		m_Traces[executing_thread_index].push_back(JobTrace(
			job_start,
			job_current,
			executing_thread_index
		));
	}

	void JobSystem::save_tracelog() {
		// this saves tracing data in a chrome-compatible format
		// more info @ https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview

		auto make_entry = [&](const JobTrace& trace) {
			using namespace std::chrono;

			nlohmann::json thread_events;

			auto start_time_ms = duration_cast<microseconds>(trace.m_StartTime - m_ApplicationStart);
			auto duration_ms   = duration_cast<microseconds>(trace.m_CurrentTime - trace.m_StartTime);

			thread_events["cat"]  = "cat";                 // categories (comma separated)
			thread_events["pid"]  = 0;                     // process ID
			thread_events["tid"]  = trace.m_ThreadIndex;   // executing thread ID
			thread_events["ts"]   = start_time_ms.count(); // tracing clock timestamp in microseconds
			thread_events["dur"]  = duration_ms.count();   // duration (specific to 'complete' events) in microseconds
			thread_events["ph"]   = "X";                   // program phase - X is 'complete' event (pp 4)
			thread_events["name"] = "-";                   // display name; we could possibly put a job type here
			thread_events["args"] = nlohmann::json();      // may hold any additional information

			return thread_events;
		};

		nlohmann::json js;

		for (const auto& thread_log : m_Traces)
			for (const auto& thread_evt : thread_log)
				js["traceEvents"].push_back(make_entry(thread_evt));

		js["displayTimeUnit"] = "ms"; // either "ms" or "ns"
		// the remaining keys are optional and not needed for our usage

		std::ofstream out("tracelog.json");
		if (!out.good())
			std::cerr << "Failed to create/open tracelog.json\n";
		else
			out	<< std::setw(2) << js;
	}

	void JobSystem::clear_tracelog() {
		for (auto& log : m_Traces)
			log.clear();
	}
}

namespace bop {
	void shutdown() {
		job::JobSystem::shutdown();
	}

	void wait_for_shutdown() {
		job::JobSystem::wait_for_shutdown();
	}
}