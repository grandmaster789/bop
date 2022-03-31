#pragma once

#include <atomic>
#include <cstdint>
#include "job.h"
#include "../util/concepts.h"

namespace bop::job {
	// singly linked list, threadsafe semantics
	class JobQueue {
	public:
		JobQueue() noexcept = default;

		JobQueue             (const JobQueue&) noexcept = delete;
		JobQueue& operator = (const JobQueue&) noexcept = delete;
		JobQueue             (JobQueue&&)      noexcept = default;
		JobQueue& operator = (JobQueue&&)      noexcept = default;
		
		uint32_t clear(); // returns the number of jobs cleared
		uint32_t size();  // returns the number of jobs currently in the queue (may be synchronized)

		// NOTE push/pop mechanics are non-owning!
		void push(Job* work);
		Job* pop(); // returns nullptr if there's nothing to return

	private:
		std::atomic_flag m_Lock = ATOMIC_FLAG_INIT; // we're using this as a non-reentrant mutex

		Job* m_Head = nullptr;
		Job* m_Tail = nullptr;

		uint32_t m_NumEntries = 0;
	};

	// very similar design, but this one doesn't have locking and thus becomes Rule-of-zero compatible
	class JobQueueNonThreadsafe {
	public:
		uint32_t clear(); // returns the number of jobs cleared
		uint32_t size(); // returns the number of jobs currently in the queue

		// NOTE push/pop mechanics are non-owning!
		void push(Job* work);
		Job* pop(); // returns nullptr if there's nothing to return

	private:
		Job* m_Head = nullptr;
		Job* m_Tail = nullptr;

		uint32_t m_NumEntries = 0;
	};
}
