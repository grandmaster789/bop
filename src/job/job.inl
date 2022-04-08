#include "job.h"
#include "job_system.h"

namespace bop::job {
	Job& Job::then(
		std::invocable auto&&   work,
		std::optional<uint32_t> thread_index
	) noexcept {
		// only constructed, not scheduled yet
		m_Continuation = JobSystem().construct(
			std::forward<decltype(work)>(work),
			nullptr,
			thread_index
		);

		return *m_Continuation;
	}
}