#include "task_system.h"

namespace bop::task {
	TaskSystem::TaskSystem(
		std::optional<uint32_t> num_threads,
		MemoryResource*         memory_resource
	) noexcept:
		m_MemoryResource(memory_resource)
	{

	}
}