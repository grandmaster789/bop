#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <thread>
#include <format>

#include "util/traits.h"
#include "util/concepts.h"

#include "job/job.h"
#include "job/job_queue.h"
#include "job/job_system.h"

#include "job/co_job.h"
#include "job/co_job_promise.h"

namespace {
	// https://en.cppreference.com/w/cpp/memory/synchronized_pool_resource
	// https://en.cppreference.com/w/cpp/memory/pool_options
	// ~ synchronized pool will accomodate allocations if possible, and
	//   use the upstream allocator for the remaining calls
	auto g_GlobalMemoryResource = std::pmr::synchronized_pool_resource(
		{
			.max_blocks_per_chunk        = 20,
			.largest_required_pool_block = 1 << 20 // == 1MB
		},
		std::pmr::new_delete_resource()
	);
}

struct Promise {
	std::coroutine_handle<> m_Handle;

	explicit Promise(std::coroutine_handle<> handle) noexcept:
		m_Handle(handle)
	{
	}

	void unhandled_exception() noexcept {
		std::terminate();
	}

	std::suspend_always initial_suspend() noexcept {
		return {};
	}

	bool resume() noexcept {
		if (m_Handle && !m_Handle.done())
			m_Handle.resume();

		return true;
	}

	// allocator overloads in order to use pmr for the coroutine stackframe allocations
	// this seems like suspicious code to me
	template <typename... Args>
	void* operator new(
		std::size_t                sz,
		std::allocator_arg_t,
		std::pmr::memory_resource* mr,
		Args&&...                  args
	) noexcept {
		auto allocatorOffset = 
			(sz + alignof(std::pmr::memory_resource*) - 1) & 
			~(alignof(std::pmr::memory_resource*) - 1);

		char* ptr = (char*)mr->allocate(allocatorOffset + sizeof(mr));

		if (ptr == nullptr)
			std::terminate();
		
		*reinterpret_cast<std::pmr::memory_resource**>(ptr + allocatorOffset) = mr;

		return ptr;
	}

	template <typename T, typename... Args>
	void* operator new(
		std::size_t                sz,
		T,
		std::allocator_arg_t,
		std::pmr::memory_resource* mr,
		Args&&...                  args
	) noexcept {
		return operator new(sz, std::allocator_arg, mr, args...);
	}

	template<typename Class, typename... Args>
	void* operator new(
		std::size_t      sz,
		Class, Args&&... args
	) noexcept {
		return operator new(
			sz, 
			std::allocator_arg,
			std::pmr::new_delete_resource(),
			args...
		);
	}

	template<typename... Args>
	void* operator new(
		std::size_t sz,
		Args&&...   args
	) noexcept {
		return operator new(
			sz, 
			std::allocator_arg,
			std::pmr::new_delete_resource(),
			args...
		);
	}

	void operator delete(
		void*       ptr,
		std::size_t sz
	) noexcept {
		auto allocatorOffset = 
			(sz + alignof(std::pmr::memory_resource*) - 1) & 
			~(alignof(std::pmr::memory_resource*) - 1
		);
		
		auto allocator = (std::pmr::memory_resource**)((char*)(ptr)+allocatorOffset);
		
		(*allocator)->deallocate(
			ptr, 
			allocatorOffset + sizeof(std::pmr::memory_resource*)
		);
	}
};

void do_your_thing() {

}

int main() {
	std::cout << "Starting application\n";

	do_your_thing();

	// creating a local instance will initialize the static class, optionally with some settings
	/*
	bop::job::JobSystem js(std::nullopt, &g_GlobalMemoryResource);
	bop::schedule([] { std::cout << "ping\n"; });

	// dummy tasks to verify that the trace data is correctly displayed by chrome
	for (int i = 0; i < 10; ++i)
		bop::schedule([=] {
			using namespace std::chrono_literals;

			for (int j = 3; j > 0; --j) {
				bop::schedule([=] {
					std::cout << std::format("** {}\n", (i * 3) + j);
					std::this_thread::sleep_for(1s);
				});
			 }
		});

	// don't immediately schedule shutdown
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(5s);
	}

	bop::schedule([] { std::cout << "Testing continuations\n"; })
		.then([] { std::cout << "3\n"; })
		.then([] { std::cout << "2\n"; })
		.then([] { std::cout << "1\n"; })
		.then([] { bop::shutdown(); });

	//bop::schedule([] { bop::shutdown(); });
	bop::wait_for_shutdown();
	*/

	std::cout << "Completed application\n";
}