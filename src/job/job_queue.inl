#pragma once

#include "job_queue.h"

namespace bop::job {
	template <typename T, bool B>
	requires std::is_base_of_v<Queueable, T>
	JobQueue<T, B>::JobQueue(const JobQueue<T, B>&) noexcept:
		m_Head      (nullptr),
		m_Tail      (nullptr),
		m_NumEntries(0)
	{
	}

	template <typename T, bool B>
	requires std::is_base_of_v<Queueable, T>
	uint32_t JobQueue<T, B>::clear() {
		uint32_t result = m_NumEntries;

		// pop() is threadsafe but not re-entrant so don't acquire the lock here
		for (auto* job = pop(); job; job = pop())
			job->get_deallocator()(static_cast<Job*>(job));

		return result;
	}

	template <typename T, bool B>
	requires std::is_base_of_v<Queueable, T>
	uint32_t JobQueue<T, B>::size() {
		if constexpr (B) {
			while (m_Lock.test_and_set(std::memory_order::acquire));
		}

		uint32_t result = m_NumEntries;

		if constexpr (B) {
			m_Lock.clear(std::memory_order::release);
		}

		return result;
	}

	template <typename T, bool B>
	requires std::is_base_of_v<Queueable, T>
	void JobQueue<T, B>::push(T* work) {
		if constexpr (B) {
			while (m_Lock.test_and_set(std::memory_order::acquire));
		}

		work->m_Next = nullptr;
		if (!m_Head)
			m_Head = work;

		if (!m_Tail)
			m_Tail = work;
		else {
			// single link push
			m_Tail->m_Next = work;
			m_Tail = work;
		}

		++m_NumEntries;

		if constexpr (B) {
			m_Lock.clear(std::memory_order::release);
		}
	}

	template <typename T, bool B>
	requires std::is_base_of_v<Queueable, T>
	T* JobQueue<T, B>::pop() {
		if constexpr (B) {
			while (m_Lock.test_and_set(std::memory_order::acquire));
		}

		auto* result = m_Head;

		if (result) {
			// single link pop
			m_Head = static_cast<T*>(result->m_Next);
			--m_NumEntries;

			if (result == m_Tail)
				m_Tail = nullptr;
		}

		if constexpr (B) {
			m_Lock.clear(std::memory_order::release);
		}

		return result;
	}
}