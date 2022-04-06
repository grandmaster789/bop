#include "job.h"

namespace bop::job {
	/***** JobDeallocator *****/
	void JobDeallocator::operator()(Job* job) noexcept {
		std::pmr::polymorphic_allocator<Job> allocator(job->m_MemoryResource);
		allocator.deallocate_object(job); // job doesn't have a destructor
	}

	/***** Job *****/
	JobDeallocator Job::get_deallocator() noexcept {
		return JobDeallocator();
	}

	void Job::operator()() noexcept {
		resume();
	}

	Job::Job(std::pmr::memory_resource* resource):
		m_MemoryResource(resource)
	{
		m_NumChildren = 1;
	}

	void Job::reset() noexcept {
		m_NumChildren  = 1;
		m_Next         = nullptr;
		m_Parent       = nullptr;
	}

	bool Job::resume() noexcept {
		if (m_WorkFnPtr)
			m_WorkFnPtr();
		else
			m_Work();

		return true;
	}

	void Job::set_next(Job* j) noexcept {
		m_Next = j;
	}

	Job* Job::get_next() const noexcept {
		return m_Next;
	}
}