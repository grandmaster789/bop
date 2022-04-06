#include "job_queue.h"

namespace bop::job {
	void JobQueue::push(Job* work) {
		while (m_Lock.test_and_set(std::memory_order::acquire));

		work->set_next(nullptr);

		if (!m_Head)
			m_Head = work;

		if (!m_Tail)
			m_Tail = work;
		else {
			// single link push
			m_Tail->set_next(work);
			m_Tail = work;
		}

		++m_NumEntries;

		m_Lock.clear(std::memory_order::release);
	}

	Job* JobQueue::pop() {
		while (m_Lock.test_and_set(std::memory_order::acquire));

		auto* result = m_Head;

		if (result) {
			// single link pop
			m_Head = result->get_next();
			--m_NumEntries;

			if (result == m_Tail)
				m_Tail = nullptr;
		}

		m_Lock.clear(std::memory_order::release);

		return result;
	}

	uint32_t JobQueue::clear() {
		uint32_t result = m_NumEntries;

		// pop() is threadsafe but not re-entrant so don't acquire the lock here
		for (auto* job = pop(); job; job = pop())
			job->get_deallocator()(static_cast<Job*>(job));

		return result;
	}

	uint32_t JobQueue::size() {
		while (m_Lock.test_and_set(std::memory_order::acquire));

		uint32_t result = m_NumEntries;

		m_Lock.clear(std::memory_order::release);

		return result;
	}

	void JobQueueNonThreadsafe::push(Job* work) {
		work->set_next(nullptr);

		if (!m_Head)
			m_Head = work;

		if (!m_Tail)
			m_Tail = work;
		else {
			// single link push
			m_Tail->set_next(work);
			m_Tail = work;
		}

		++m_NumEntries;
	}

	Job* JobQueueNonThreadsafe::pop() {
		auto* result = m_Head;

		if (result) {
			// single link pop
			m_Head = result->get_next();
			--m_NumEntries;

			if (result == m_Tail)
				m_Tail = nullptr;
		}

		return result;
	}

	uint32_t JobQueueNonThreadsafe::clear() {
		uint32_t result = m_NumEntries;

		for (auto* job = pop(); job; job = pop())
			job->get_deallocator()(static_cast<Job*>(job));

		return result;
	}

	uint32_t JobQueueNonThreadsafe::size() {
		uint32_t result = m_NumEntries;

		return result;
	}
}