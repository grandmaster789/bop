#include "job_trace.h"

namespace bop::job {
	JobTrace::JobTrace(
		Timepoint start_time,
		Timepoint current_time,
		uint32_t  thread_index
	) noexcept:
		m_StartTime  (start_time),
		m_CurrentTime(current_time),
		m_ThreadIndex(thread_index)
	{
	}
}