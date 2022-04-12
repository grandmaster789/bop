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
#include "job/co_generator.h"

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

class Task { 
public:
	struct promise_type {
		using Handle = std::coroutine_handle<promise_type>;

		std::suspend_always initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void unhandled_exception() {}
		void return_void() {}

		Task get_return_object() {
			return Task(Handle::from_promise(*this));
		}
	};

	explicit Task(promise_type::Handle handle) :
		m_Handle(handle)
	{
	}

	Task             (const Task&) = delete;
	Task& operator = (const Task&) = delete;
	Task             (Task&& t) noexcept = default;
	Task& operator = (Task&& t) noexcept = default;

	void destroy() { m_Handle.destroy(); }
	void resume() { m_Handle.resume(); }

private:
	promise_type::Handle m_Handle;
};

Task do_your_thing() {
	std::cout << "ping\n";
	co_return;
}

bop::job::Generator<int> five() {
	int value = 0;

	while (true) {
		co_yield value;

		if (++value > 5)
			value = 0;
	}
}

int main() {
	std::cout << "Starting application\n";

	Task t = do_your_thing();
	t.resume();

	{
		auto g = five();
		int x = 0;

		while (x < 10) {
			x += g.get_next();
			std::cout << x << " ";
		}

		std::cout << "\n";
	}

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