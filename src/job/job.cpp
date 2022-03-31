#include "job.h"

namespace bop::job {
	/***** JobDeallocator *****/
	void JobDeallocator::operator()(Job* job) noexcept {
		std::pmr::polymorphic_allocator<Job> allocator(job->m_MemoryResource);
		job->~Job(); // explicitly call the destructor
		allocator.deallocate(job, 1);
	}

	/***** Job *****/
	JobDeallocator Job::get_deallocator() noexcept {
		return JobDeallocator();
	}

	void Job::operator()() noexcept {
		resume();
	}

	bool Job::is_function() noexcept {
		return m_IsFunction;
	}

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

	void Job::set_next(Job* j) noexcept {
		m_Next = j;
	}

	Job* Job::get_next() const noexcept {
		return m_Next;
	}
}