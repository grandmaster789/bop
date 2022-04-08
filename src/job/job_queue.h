#pragma once

#include <atomic>
#include <cstdint>

#include "../util/concepts.h"

namespace bop::job {
	class Job;

	// intrusive singly linked list, non-owning, threadsafe semantics
	class JobQueue {
	public:
		JobQueue() noexcept = default;

		// the atomic flag makes it kind of difficult to put this in a vector...
		JobQueue             (const JobQueue&) = delete; 
		JobQueue& operator = (const JobQueue&) = delete;
		JobQueue             (JobQueue&&)      = delete;
		JobQueue& operator = (JobQueue&&)      = delete;
		
		// NOTE push/pop mechanics are non-owning!
		//      by using pointers we also support derived types
		void push(Job* work);
		Job* pop(); // returns nullptr if there's nothing to return

		uint32_t clear(); // returns the number of jobs cleared
		uint32_t size();  // returns the number of jobs currently in the queue (may be synchronized)

	private:
		// we're using this flag as a non-reentrant mutex; as a consequence, this object must be memory-stable
		std::atomic_flag m_Lock = ATOMIC_FLAG_INIT; 

		Job* m_Head = nullptr;
		Job* m_Tail = nullptr;

		uint32_t m_NumEntries = 0;
	};

	// very similar design, but this one doesn't have locking
	// the non-owning nature actually makes this copyable 
	// (you probably shouldn't though)
	class JobQueueNonThreadsafe {
	public:
		JobQueueNonThreadsafe() = default;

		void push(Job* work);
		Job* pop(); // returns nullptr if there's nothing to return

		uint32_t clear(); // returns the number of jobs cleared
		uint32_t size();  // returns the number of jobs currently in the queue

	private:
		Job* m_Head = nullptr;
		Job* m_Tail = nullptr;

		uint32_t m_NumEntries = 0;
	};
}
