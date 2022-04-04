#pragma once

#include <chrono>
#include <cstdint>

namespace bop::job {
	struct JobTrace {
		using Clock     = std::chrono::high_resolution_clock;
		using Timepoint = Clock::time_point;

		JobTrace(
			Timepoint start_time,
			Timepoint current_time,
			uint32_t  thread_index
		) noexcept;

		Timepoint m_StartTime;
		Timepoint m_CurrentTime;
		uint32_t  m_ThreadIndex;
	};
}