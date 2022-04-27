#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <memory_resource>
#include <atomic>
#include <thread>
#include <type_traits>

#include "task.h"

namespace bop::task {
	class TaskSystem {
	public:
		using MemoryResource = std::pmr::memory_resource;

		// uses the PMR composable memory allocation backend
		TaskSystem(
			std::optional<uint32_t> num_threads     = std::nullopt,                   // by default this will use the hardware concurrency
			MemoryResource*         memory_resource = std::pmr::new_delete_resource()
		) noexcept;

		TaskSystem             (const TaskSystem&)     = delete;
		TaskSystem& operator = (const TaskSystem&)     = delete;
		TaskSystem             (TaskSystem&&) noexcept = default;
		TaskSystem& operator = (TaskSystem&&) noexcept = default;

		auto schedule(std::invocable auto&& fn);

	private:
		MemoryResource*          m_MemoryResource = nullptr;
		std::vector<std::thread> m_WorkerThreads;
		std::atomic<uint32_t>    m_NumThreads = 0;
	};
}

#include "task_system.inl"