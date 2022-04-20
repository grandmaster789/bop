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

#include "job/job.h"
#include "job/job_queue.h"
#include "job/job_system.h"

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

/*
	Current target code:

	bop::Future<int> make_int() {
		co_return 444;
	}

	bop::Future<int> compute() {
		int a = co_await make_int(); // should schedule with the global scheduler
		int b = co_await make_int(); // should schedule with the global scheduler

		co_return a + b;
	}

	int main() {
		std::cout << compute().get() << "\n"; // should block until executed by the global jobsystem

		bop::shutdown();
		bop::wait_for_shutdown();
	}
*/

namespace bop {
	// condition-variable (push notitication) based future
	// alternatively we could use the scheduler to poll for a result
	// this has a fair amount of overhead, but is suitable for long-running tasks
	template <typename T>
	class AsyncPushFuture {
	public:
		AsyncPushFuture() noexcept = default;

		void set(T value) 
			requires std::movable<T>
		{
			std::lock_guard lock(m_Mutex);
			m_Payload = std::move(value);
			m_Condition.notify_one();
		}

		void set(const T& value)
			requires !std::movable<T>
		{
			std::lock_guard lock(m_Mutex);
			m_Payload = value;
			m_Condition.notify_one();
		}

		void set_exception(std::exception_ptr ex) {
			std::lock_guard lock(m_Mutex);
			m_Payload = std::move(ex);
			m_Condition.notify_one();
		}

		const T& get() const {
			while (true) {
				switch (m_Payload.index()) {
				case 0: // monostate -- wait until something else was set
				{
					std::unique_lock lock(m_Mutex);
					m_Condition.wait(lock, [&] { return m_Payload.index() != 0; });
				}
					break;

				case 1: // value -- return it
					return std::get<T>(m_Payload);

				case 2: // exception_ptr -- rethrow
					std::rethrow_exception(std::get<std::exception_ptr>(m_Payload));
				}
			}
		}

		T&& get() {
			while (true) {
				switch (m_Payload.index()) {
				case 0:
				{
					std::unique_lock lock(m_Mutex);
					m_Condition.wait(lock, [&] { return m_Payload.index() != 0; });
				}
				case 1:
				{
					return std::move(std::get<T>(m_Payload));
				}

				case 2:
					std::rethrow_exception(std::get<std::exception_ptr>(m_Payload));
				}
			}
		}

	private:
		mutable std::mutex              m_Mutex;
		mutable std::condition_variable m_Condition;

		std::variant<std::monostate, T, std::exception_ptr> m_Payload;
	};

	template <typename T>
	class Future {
	public:
		struct promise_type {
			Future<T> get_return_object() { return Future<T>(this); }

			void return_value(T value)    { m_State = value;}
			void unhandled_exception()    { m_State = std::current_exception(); }
			auto initial_suspend()        { return std::suspend_always(); } // if you want to lazily execute the coroutine body, start in a suspended state
			auto final_suspend() noexcept { return std::suspend_always(); } // if you want to access the promise after the coroutine is done, suspend it in the end

			std::variant<std::monostate, T, std::exception_ptr> m_State;
		};

		using Handle = std::coroutine_handle<promise_type>;

		explicit Future(promise_type* pm):
			m_Handle(Handle::from_promise(*pm))
		{
		}

		~Future() {
			if (m_Handle)
				m_Handle.destroy();
		}

		bool await_ready() { return false; }
		void await_suspend(std::coroutine_handle<promise_type> caller) noexcept {}
		T    await_resume() {
			while (m_Handle && !m_Handle.done())
				m_Handle.resume();

			auto& pm = m_Handle.promise();

			if (auto* ex = std::get_if<std::exception_ptr>(&pm.m_State))
				std::rethrow_exception(*ex);
			else if (pm.m_State.index() == 0)
				throw std::runtime_error("No value was set in the promise");
			else
				return std::get<T>(pm.m_State);
		}

		T get() {
			return await_resume();
		}

	private:
		Handle m_Handle;
	};
}

bop::Future<int> make_int() {
	co_return 444;
}

bop::Future<int> compute() {
	int a = co_await make_int();
	int b = co_await make_int();
	
	co_return a + b;
}

int main() {
	std::cout << "Starting application\n";

	/*
	bop::Future<int> x;

	std::jthread thd([&] {
		using namespace std::chrono_literals;
		std::cout << "Sleeping\n";
		std::this_thread::sleep_for(3s);
		std::cout << "Setting\n";
		
		try {
			throw std::runtime_error("f00");
		}
		catch (...) {
			x.set_exception(std::current_exception());
		}
	});

	std::cout << "Waiting\n";

	try {
		std::cout << x.get() << "\n";
	}
	catch (std::exception& ex) {
		std::cout << "Caught: " << ex.what() << "\n";
	}
	*/

	std::cout << make_int().get() << "\n";
	std::cout << compute().get() << "\n";

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
		std::cout << "Sleeping main thread\n";
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(5s);
		std::cout << "Continuing main thread\n";
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