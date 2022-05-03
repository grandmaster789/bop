#include "task_scheduler.h"

namespace bop::task {
	TaskScheduler::TaskScheduler(
		std::optional<uint32_t> num_threads,
		MemoryResource*         memory_resource
	) noexcept:
		m_MemoryResource(memory_resource)
	{

	}
}