#include "job_system.h"

#include <iostream>
#include <fstream>

namespace bop::job {
	namespace {
		void save_trace(
			std::ofstream&     out,
			const std::string& category,
			uint64_t           pid,                 // always 0
			uint64_t           executing_thread_id,
			uint64_t           start_timestamp,     // in milliseconds
			int64_t            duration,            // in milliseconds
			const std::string& ph,                  // always 'X'
			const std::string& job_name,
			const std::string& args
		) {
			out << "{";
			out << "\"cat\": "   << category << ", ";
			out << "\"pid\": "   << pid << ", ";
			out << "\"tid\": "   << executing_thread_id << ", ";
			out << "\"ts\":  "   << std::to_string(start_timestamp) << ", ";
			out << "\"dur\": "   << std::to_string(duration) << ", ";
			out << "\"ph\": "    << ph << ", ";
			out << "\"name\": "  << job_name << ", ";
			out << "\"args\": {" << args << "}";
			out << "}";
		}

		void save_log(
			const JobSystem::TimePoint&   application_start,
			const JobSystem::TracingData& data,
			const JobSystem::TypeMapping& type_mapping
		) {
			// chrome-compatible tracing data
			std::ofstream out("trace.json");

			if (!out.good()) {
				std::cerr << "Failed to open trace.json\n";
				return;
			}

			// TODO FIXME replace this with nlohmann-json
			out << "{\n";
			out << "\"traceEvents\":[\n";

			bool add_comma_newline = false;

			for (size_t i = 0; i < data.size(); ++i) {
				for (const auto& evt : data[i]) {
					if (add_comma_newline)
						out << ",\n";

					std::string type_name = "?";
					auto it = type_mapping.find(evt.m_ThreadType.m_Type);
					if (it != std::end(type_mapping))
						type_name = it->second;

					using namespace std::chrono;

					save_trace(
						out,
						"\tcat\"",
						0,
						static_cast<uint32_t>(evt.m_ExecutingThread.m_Index),
						duration_cast<milliseconds>(evt.m_StartTime - application_start).count(),
						duration_cast<milliseconds>(evt.m_StopTime - evt.m_StartTime).count(),
						"\"X\"",
						"\"" + type_name + "\"",
						"\"id\": " + std::to_string(evt.m_ThreadID.m_Id)
					);

					add_comma_newline = true;
				}
			}

			out << "],\n";
			out << "\"displayTimeUnit\": \"ms\"\n";
			out << "}\n";
		}
	}

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
		m_NumThreads     = num_threads.m_Count;

		if (m_NumThreads < 0)
			m_NumThreads = std::thread::hardware_concurrency();

		if (m_NumThreads == 0)
			m_NumThreads = 1; // always have at least one worker

		// initialize queue logic for all worker threads
		for (uint32_t i = 0; i < m_NumThreads; ++i) {
			m_GlobalQueues.push_back(JobQueue<JobBase>());
			m_LocalQueues.push_back(JobQueue<JobBase>());
			m_Mutexes.emplace_back();
		}

		m_Traces.resize(m_NumThreads, std::pmr::vector<JobTrace>(m_MemoryResource));

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
		m_Shutdown = true;
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

	bool JobSystem::child_completed(JobBase* job) noexcept {
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
			static std::atomic<uint32_t> remaining_threads = m_NumThreads.load();
			--remaining_threads;
			while (remaining_threads > 0)
				std::this_thread::yield();
		}

		// set up work-stealing indices
		uint32_t steal_from = static_cast<uint32_t>(idx.m_Index + 1) % m_NumThreads.load();

		auto start_time_point = Clock::now();

		std::unique_lock lock(*m_Mutexes[l_ThreadIndex.m_Index]);

		// do work while we're not done
		while (!m_Shutdown) {
			// prioritize local queue over the global one (should have less contention)
			l_CurrentJob = m_LocalQueues[l_ThreadIndex.m_Index].pop();

			if (!l_CurrentJob)
				l_CurrentJob = m_GlobalQueues[l_ThreadIndex.m_Index].pop();

			// if we still don't have any work yet try to steal it from another global queue
			uint32_t numStealAttempts = m_NumThreads - 1;
			while (!l_CurrentJob && (--numStealAttempts > 0)) {
				if (++steal_from >= m_NumThreads)
					steal_from = 0;

				l_CurrentJob = m_GlobalQueues[steal_from].pop();
			}

			// if we have a job, execute it
			if (l_CurrentJob) {
				TimePoint        start_time;
				util::ThreadType type;
				util::ThreadID   id;

				if constexpr (k_EnableTracing) {
					if (m_DoLogging)
						start_time = Clock::now();

					type = l_CurrentJob->m_ThreadType;
					id   = l_CurrentJob->m_ThreadID;
				}

				// before doing the work, remember if this was a regular function or a coroutine
				// (in the coro case the job may destroy itself)
				bool is_function = l_CurrentJob->is_function(); 

				(*l_CurrentJob)(); // do the actual work

				if constexpr (k_EnableTracing) {
					if (m_DoLogging) {
						append_trace(
							start_time,
							Clock::now(),
							is_function, // functions are complete after doing the work; coroutines may have suspended
							l_ThreadIndex,
							type,
							id
						);
					}
				}

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
		m_GlobalQueues[l_ThreadIndex.m_Index].clear();
		m_LocalQueues[l_ThreadIndex.m_Index].clear();
		l_RecyclingBin.clear();
		l_GarbageBin.clear();

		// the last worker may save the trace report
		uint32_t active_workers = m_NumThreads.fetch_sub(1);
		if (active_workers == 1) {
			if constexpr (k_EnableTracing) {
				if (m_DoLogging) {
					save_log(m_StartTime, m_Traces, m_TypeNames);
					clear_logs();
				}
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

	JobBase* JobSystem::get_current_work() noexcept {
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

	void JobSystem::enable_logging() {
		m_DoLogging = true;
	}

	void JobSystem::disable_logging() {
		if (m_DoLogging)
			save_log(
				m_StartTime,
				m_Traces,
				m_TypeNames
			);

		m_DoLogging = false;
	}

	void JobSystem::clear_logs() {
		for (auto& log : m_Traces)
			log.clear();
	}

	JobSystem::TimePoint JobSystem::get_start_time() {
		return m_StartTime;
	}

	const JobSystem::TypeMapping& JobSystem::get_type_mapping() const {
		return m_TypeNames;
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

	bool JobSystem::schedule_work(
		JobBase*  work,
		util::Tag tag
	) noexcept {
		static thread_local util::ThreadIndex tidx(0);

		// if a specific tag was assigned, add it to a tag queue
		if (tag.m_Value >= 0) {
			if (!m_TagQueues.contains(tag))
				m_TagQueues[tag] = std::make_unique<JobQueue<JobBase>>();

			m_TagQueues[tag]->push(work);

			return false;
		}

		// if no specific execution thread was set, plonk it any global queue
		if (
			(work->m_ThreadIndex.m_Index < 0) ||
			(work->m_ThreadIndex.m_Index >= static_cast<int>(m_NumThreads))
		) {
			++tidx.m_Index;
			if (tidx.m_Index >= m_NumThreads)
				tidx.m_Index = 0;

			m_GlobalQueues[tidx.m_Index].push(work);
			m_WaitCondition.notify_all(); // wake up the thread (if it was waiting)

			return true;
		}

		// if a specific execution thread was set, plonk it in the appropriate local queue
		m_LocalQueues[work->m_ThreadIndex.m_Index].push(work);
		m_WaitCondition.notify_all();

		return true;
	}

	uint32_t JobSystem::schedule_tag(
		util::Tag tag,
		JobBase*  parent,
		int32_t   num_children
	) noexcept {
		if (!m_TagQueues.contains(tag))
			return 0; // if the queue doesn't exist, no jobs can be scheduled

		auto*  queue    = m_TagQueues[tag].get();

		// by keeping a count of how many we had at the beginning we can allow additional jobs
		// to be added to the queue without risking an infinite cycle
		size_t num_jobs = queue->size(); 

		// if a parent job was specified, add the number of child jobs to their total count
		if (parent) {
			if (num_children < 0)
				num_children = static_cast<int32_t>(num_jobs);

			parent->m_NumChildren.fetch_add(static_cast<int>(num_children));
		}

		size_t scheduled_jobs = 0;

		while (num_jobs > 0) {
			JobBase* work = queue->pop();

			if (!work)
				return scheduled_jobs; // this shouldn't happen, but jic

			work->m_Parent = parent;

			schedule_work(work);
			--num_jobs;
			++scheduled_jobs;
		}

		return scheduled_jobs;
	}

	void JobSystem::append_trace(
		const TimePoint&  start,
		const TimePoint&  stop,
		bool              has_completed,
		util::ThreadIndex executing_thread,
		util::ThreadType  job_type,
		util::ThreadID    id
	) {
		m_Traces[executing_thread.m_Index].emplace_back(
			start,
			stop,
			has_completed,
			executing_thread,
			job_type,
			id
		);
	}
}

namespace bop {
	job::JobBase* current_work() {
		return job::JobSystem::get_current_work();
	}
}