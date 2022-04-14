#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <thread>
#include <format>

#include "util/traits.h"
#include "util/concepts.h"
#include "util/cacheline.h"
#include "util/spinlock.h"

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

	~Task() {
		// final handle should destroy the coroutine (sounds like refcounted semantics)
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

struct Sleeper {
	constexpr bool await_ready() const noexcept { return false; }

	void await_suspend(std::coroutine_handle<> h) const {
		auto t = std::jthread([h] {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1s);
			h.resume(); // resumes on another thread
		});
	}

	constexpr void await_resume() const noexcept {}
};

Task sleepy() {
	Sleeper s;
	std::cout << "Before\n";
	co_await s;
	std::cout << "After\n";
}

int main() {
	std::cout << "Starting application\n";

	Task t = do_your_thing();
	t.resume();

	{
		for (auto i : bop::job::range(5, 11))
			std::cout << i << " ";

		std::cout << "\n";
	}

	sleepy().resume();

	std::cout << std::boolalpha << bop::util::is_awaiter_v<Sleeper> << "\n";
	std::cout << std::boolalpha << bop::util::is_awaiter_v<Task> << "\n";

	std::cout << "Cacheline: " << bop::util::hardware_constructive_interference_size << "\n";

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