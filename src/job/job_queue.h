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

		// the atomic flag makes it kind of difficult to put this in a vector...
		JobQueue             (const JobQueue&) noexcept = delete; 
		JobQueue& operator = (const JobQueue&) noexcept = delete;
		JobQueue             (JobQueue&&)      noexcept = delete;
		JobQueue& operator = (JobQueue&&)      noexcept = delete;
		
		uint32_t clear(); // returns the number of jobs cleared
		uint32_t size();  // returns the number of jobs currently in the queue (may be synchronized)

		// NOTE push/pop mechanics are non-owning!
		void push(Job* work);
		Job* pop(); // returns nullptr if there's nothing to return

	private:
		// we're using this flag as a non-reentrant mutex; as a consequence, this object must be memory-stable
		std::atomic_flag m_Lock = ATOMIC_FLAG_INIT; 

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
