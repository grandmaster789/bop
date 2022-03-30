#include "job.h"

namespace bop::job {
	/***** JobDeallocator *****/
	void JobDeallocator::operator()(Job* job) noexcept {
		std::pmr::polymorphic_allocator<Job> allocator(job->m_MemoryResource);
		job->~Job(); // explicitly call the destructor
		allocator.deallocate(job, 1);
	}

	/***** JobBase *****/
	JobDeallocator JobBase::get_deallocator() noexcept {
		return JobDeallocator();
	}

	void JobBase::operator()() noexcept {
		resume();
	}

	bool JobBase::is_function() noexcept {
		return m_IsFunction;
	}

	/***** Job *****/
	Job::Job(std::pmr::memory_resource* resource):
		m_MemoryResource(resource)
	{
		m_NumChildren = 1;
		m_IsFunction  = true;
	}

	void Job::reset() noexcept {
		m_Next         = nullptr;
		m_NumChildren  = 1;
		m_Parent       = nullptr;
		m_Continuation = nullptr;

		m_ThreadIndex = util::ThreadIndex();
		m_ThreadType  = util::ThreadType();
		m_ThreadID    = util::ThreadID();
	}

	bool Job::resume() noexcept {
		m_NumChildren = 1;

		if (m_WorkFnPtr)
			m_WorkFnPtr();
		else
			m_Work();

		return true;
	}

	bool Job::deallocate() noexcept {
		return true;
	}

	/***** JobTrace *****/
	JobTrace::JobTrace(
		TimePoint         start_time,
		TimePoint         stop_time,
		bool              completed,
		util::ThreadIndex executing_on_thread,
		util::ThreadType  thread_type,
		util::ThreadID    thread_id
	):
		m_StartTime      (std::move(start_time)),
		m_StopTime       (std::move(stop_time)),
		m_Completed      (completed),
		m_ExecutingThread(executing_on_thread),
		m_ThreadType     (thread_type),
		m_ThreadID       (thread_id)
	{
	}
}