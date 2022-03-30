#pragma once

#include <atomic>
#include <cstdint>
#include "job.h"
#include "../util/concepts.h"

namespace bop::job {
	// singly linked list, optionally threadsafe semantics
	template <
		typename T            = Queueable,
		bool     t_Threadsafe = true
	>
	requires std::is_base_of_v<Queueable, T>
	class JobQueue {
	public:
		friend class JobSystem;

		JobQueue() noexcept = default;

		// NOTE we can't default-copy because of the atomic flag; it is used during jobsystem init
		JobQueue             (const JobQueue&) noexcept;
		JobQueue& operator = (const JobQueue&) noexcept = default;
		JobQueue             (JobQueue&&)      noexcept = default;
		JobQueue& operator = (JobQueue&&)      noexcept = default;
		
		uint32_t clear(); // returns the number of jobs cleared
		uint32_t size();  // returns the number of jobs currently in the queue (may be synchronized)

		void push(T* work);
		T*   pop(); // returns nullptr if there's nothing to return

	private:
		std::atomic_flag m_Lock = ATOMIC_FLAG_INIT;

		T* m_Head = nullptr;
		T* m_Tail = nullptr;

		uint32_t m_NumEntries = 0;
	};
}

#include "job_queue.inl"