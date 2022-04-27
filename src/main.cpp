#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <thread>
#include <format>

#include "util/traits.h"
#include "util/concepts.h"
#include "util/cacheline.h"
#include "util/spinlock.h"
#include "util/manual_lifetime.h"
#include "util/function.h"

#include "job/job.h"
#include "job/job_queue.h"
#include "job/job_system.h"

#include "task/task.h"

#include "job/co_job.h"
#include "job/co_job_promise.h"
#include "job/co_generator.h"

#include <mutex>
#include <variant>

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

bop::job::CoJob<void> test_co_print() {
	int i = 1;
	std::cout << "Testing coro print\n";
	co_return;
}

int main() {
	std::cout << "Starting application\n";

	// creating a local instance will initialize the static class, optionally with some settings
	bop::job::JobSystem js(std::nullopt, &g_GlobalMemoryResource);
	bop::schedule([] { std::cout << "ping\n"; });

	// dummy tasks to verify that the trace data is correctly displayed by chrome
	/*
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
		std::cout << "Sleeping main thread\n";
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(5s);
		std::cout << "Continuing main thread\n";
	}
	*/

	// coroutines


	bop::schedule([] { std::cout << "Testing continuations\n"; })
		.then([] { std::cout << "3\n"; })
		.then([] { std::cout << "2\n"; })
		.then([] { std::cout << "1\n"; })
		.then([] { bop::shutdown(); });

	//bop::schedule([] { bop::shutdown(); });
	bop::wait_for_shutdown();

	std::cout << "Completed application\n";
}