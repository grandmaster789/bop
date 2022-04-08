#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory_resource>
#include <optional>

namespace bop::job {
	class Job {
	public:
		friend class JobSystem;
		friend class JobQueue;
		friend class JobQueueNonThreadsafe;
		friend class JobDeallocator; // because this needs to access the memory resource

		// (pmr allocation/deallocation is done in JobSystem)
		Job() = default;

		void operator()() noexcept;

		void reset() noexcept;

		inline Job& then(
			std::invocable auto&&   work,
			std::optional<uint32_t> thread_index = std::nullopt
		) noexcept;

	protected:
		std::atomic<uint32_t>   m_NumChildren    = 1;
		Job*                    m_Parent         = nullptr;
		Job*                    m_Continuation   = nullptr;
		Job*                    m_Next           = nullptr; // intrusive singly linked
		std::optional<uint32_t> m_ThreadIndex    = std::nullopt;

		std::function<void()>   m_Work;
	};
}

#include "job.inl"