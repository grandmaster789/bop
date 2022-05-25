#pragma once

#include <atomic>
#include "task.h"

namespace bop::task {
	// NOTE this is an intrusive, non-owning, threadsafe structure
	//      in some cases threadsafety is not required
	class TaskQueue {
	public:
		TaskQueue() noexcept = default;
		~TaskQueue() = default;

		TaskQueue             (const TaskQueue&)     = delete;
		TaskQueue& operator = (const TaskQueue&)     = delete;
		TaskQueue             (TaskQueue&&) noexcept = delete;
		TaskQueue& operator = (TaskQueue&&) noexcept = delete;

		void  push(Task* t) noexcept;
		Task* pop() noexcept;

		uint32_t clear();
		uint32_t size() const noexcept;

	private:
		// performs acquire/release on construction/destruction
		struct LockGuard {
			LockGuard(std::atomic_flag& flag);
			~LockGuard();

			LockGuard             (const LockGuard&)     = delete;
			LockGuard& operator = (const LockGuard&)     = delete;
			LockGuard             (LockGuard&&) noexcept = delete;
			LockGuard& operator = (LockGuard&&) noexcept = delete;

			std::atomic_flag& m_Mutex;
		};

		mutable std::atomic_flag m_Mutex = ATOMIC_FLAG_INIT; // we're using this flag as a non-reentrant mutex; as a consequence, this object must be memory-stable

		Task* m_Head = nullptr;
		Task* m_Tail = nullptr;

		uint32_t m_NumEntries = 0;
	};
}