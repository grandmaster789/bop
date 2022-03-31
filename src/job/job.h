#pragma once

#include <atomic>
#include <memory_resource>
#include <chrono>

#include "../util/thread_types.h"
#include "../util/function.h"

namespace bop::job {
	class Job;

	struct JobDeallocator {
		virtual void operator()(Job* job) noexcept;
	};

	class Job {
	public:
		friend class JobSystem;
		friend class JobDeallocator; // because this needs to access the memory resource

		Job(std::pmr::memory_resource* resource);

		virtual JobDeallocator get_deallocator() noexcept;

		void operator()() noexcept;

		bool is_function() noexcept; // code smell... probably should use variant instead

		void reset() noexcept;
		bool resume() noexcept;
		bool deallocate() noexcept;

		void set_next(Job* j) noexcept;
		Job* get_next() const noexcept;

	protected:
		std::atomic<int>           m_NumChildren    = 0;
		Job*                       m_Parent         = nullptr;
		Job*                       m_Next           = nullptr; // singly linked
		Job*                       m_Continuation   = nullptr;
		bool                       m_IsFunction     = false;
		util::ThreadIndex          m_ThreadIndex;
		std::pmr::memory_resource* m_MemoryResource = nullptr;

		util::Function          m_Work;
		util::void_function_ptr m_WorkFnPtr;
	};
}