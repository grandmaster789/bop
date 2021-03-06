#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <memory_resource>
#include <atomic>
#include <thread>
#include <type_traits>

#include "task.h"
#include "task_queue.h"

namespace bop::task {
	class TaskScheduler {
	public:
		using MemoryResource = std::pmr::memory_resource;

		// uses the PMR composable memory allocation backend
		TaskScheduler(
			std::optional<uint32_t> num_threads     = std::nullopt,                   // by default this will use the hardware concurrency
			MemoryResource*         memory_resource = std::pmr::new_delete_resource()
		) noexcept;

		TaskScheduler             (const TaskScheduler&)     = delete;
		TaskScheduler& operator = (const TaskScheduler&)     = delete;
		TaskScheduler             (TaskScheduler&&) noexcept = default;
		TaskScheduler& operator = (TaskScheduler&&) noexcept = default;

	private:
		Task* allocate_task();

		MemoryResource*          m_MemoryResource = nullptr;
		std::vector<std::thread> m_WorkerThreads;
		std::atomic<uint32_t>    m_NumThreads = 0;

		// thread-local resources
		// [NOTE] this complicates testing; do we even want these? 
		static thread_local TaskQueue m_Garbage;
		static thread_local TaskQueue m_Recycle;
	};
}

#include "task_scheduler.inl"