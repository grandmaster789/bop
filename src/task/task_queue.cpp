#include "task_queue.h"

namespace bop::task {
	void TaskQueue::push(Task* t) noexcept {
		LockGuard guard(m_Mutex);

		t->m_NextLink = nullptr; // clear any previous link

		if (!m_Head)
			m_Head = t;

		if (!m_Tail)
			m_Tail = t;
		else {
			// single link push
			m_Tail->m_NextLink = t;
			m_Tail = t;
		}

		++m_NumEntries;
	}

	Task* TaskQueue::pop() noexcept {
		LockGuard guard(m_Mutex);

		auto* result = m_Head; // NOTE may be nullptr

		if (result) {
			// single link pop
			m_Head = result->m_NextLink;
			--m_NumEntries;

			if (result == m_Tail)
				m_Tail = nullptr;
		}

		return result;
	}

	uint32_t TaskQueue::clear() {
		LockGuard guard(m_Mutex);

		uint32_t result = m_NumEntries;

		m_Head = nullptr;
		m_Tail = nullptr;
		m_NumEntries = 0;

		return result;
	}

	uint32_t TaskQueue::size() const noexcept {
		LockGuard guard(m_Mutex);
		return m_NumEntries;
	}

	/***** LockGuard *****/

	TaskQueue::LockGuard::LockGuard(std::atomic_flag& flag):
		m_Mutex(flag)
	{
		while (m_Mutex.test_and_set(std::memory_order::acquire)) [[unlikely]];
	}

	TaskQueue::LockGuard::~LockGuard() {
		m_Mutex.clear(std::memory_order::release);
	}
}