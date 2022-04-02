#include "job_trace.h"

namespace bop::job {
	JobTrace::JobTrace(
		Timepoint start_time,
		Timepoint current_time,
		bool      completed,
		uint32_t   thread_index
	) noexcept:
		m_StartTime  (start_time),
		m_CurrentTime(current_time),
		m_Completed  (completed),
		m_ThreadIndex(thread_index)
	{
	}
}