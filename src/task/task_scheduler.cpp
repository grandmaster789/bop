#include "task_scheduler.h"

namespace bop::task {
	TaskScheduler::TaskScheduler(
		std::optional<uint32_t> num_threads,
		MemoryResource*         memory_resource
	) noexcept:
		m_MemoryResource(memory_resource)
	{

	}

	Task* TaskScheduler::allocate_task() {
		// see if we can re-use an old task
		if (Task* x = m_Recycle.pop()) {
			x->reset();
			return x;
		}
		else {

		}
	}
}