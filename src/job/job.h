#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory_resource>
#include <optional>

namespace bop::job {
	class Job {
	public:
		using void_function_ptr = void(*)();

		friend class JobSystem;
		friend class JobQueue;
		friend class JobQueueNonThreadsafe;
		friend class JobDeallocator; // because this needs to access the memory resource

		// (pmr allocation/deallocation is done in JobSystem)
		Job() = default;

		void operator()() noexcept;

		void reset() noexcept;

	protected:
		std::atomic<int>        m_NumChildren    = 1;
		Job*                    m_Parent         = nullptr;
		//Job*                    m_Continuation   = nullptr;
		Job*                    m_Next           = nullptr; // singly linked
		std::optional<uint32_t> m_ThreadIndex    = std::nullopt;

		std::function<void()>   m_Work;
		void_function_ptr       m_WorkFnPtr = nullptr;
	};
}