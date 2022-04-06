#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory_resource>
#include <optional>

namespace bop::job {
	class Job;

	struct JobDeallocator {
		virtual void operator()(Job* job) noexcept; // pmr 
	};

	class Job {
	public:
		using void_function_ptr = void(*)();

		friend class JobSystem;
		friend class JobDeallocator; // because this needs to access the memory resource

		// pmr-aware allocation (just stores the resource pointer)
		// (allocation is done in JobSystem::allocate)
		Job(std::pmr::memory_resource* resource); 

		virtual JobDeallocator get_deallocator() noexcept;

		void operator()() noexcept;

		void reset() noexcept;
		bool resume() noexcept;

		void set_next(Job* j) noexcept;
		Job* get_next() const noexcept;

	protected:
		std::atomic<int>           m_NumChildren    = 0;
		Job*                       m_Parent         = nullptr;
		Job*                       m_Next           = nullptr; // singly linked
		std::optional<uint32_t>    m_ThreadIndex    = std::nullopt;
		std::pmr::memory_resource* m_MemoryResource = nullptr;

		std::function<void()>      m_Work;
		void_function_ptr          m_WorkFnPtr;
	};
}