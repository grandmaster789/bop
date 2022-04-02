#pragma once

#include <chrono>
#include <cstdint>

namespace bop::job {
	class JobTrace {
	public:
		using Clock     = std::chrono::high_resolution_clock;
		using Timepoint = Clock::time_point;

		JobTrace(
			Timepoint start_time,
			Timepoint current_time,
			bool      completed,
			uint32_t   thread_index
		) noexcept;

	private:
		Timepoint m_StartTime;
		Timepoint m_CurrentTime;
		bool      m_Completed;
		uint32_t  m_ThreadIndex;
	};
}