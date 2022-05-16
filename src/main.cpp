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
#include "util/scope_guard.h"
#include "util/typelist.h"

#include "job/job.h"
#include "job/job_queue.h"
#include "job/job_system.h"

#include "task/task.h"
#include "task/task_scheduler.h"

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

template <typename>
struct CaptureSlots;

namespace detail {
	template <class Fn, class Tuple, std::size_t... I>
	constexpr auto apply_unwrap_impl(
		Fn&&    fn,
		Tuple&& tup,
		std::index_sequence<I...>
	) {
		return std::invoke(
			std::forward<Fn>(fn),
			(*std::get<I>(std::forward<Tuple>(tup)))...
		);
	}
}

template <class Fn, class Tuple>
constexpr auto apply_unwrap(
	Fn&&    fn, 
	Tuple&& tup
) {
	return detail::apply_unwrap_impl(
		std::forward<Fn>(fn), 
		std::forward<Tuple>(tup),
		std::make_index_sequence<
			std::tuple_size_v<
				std::remove_reference_t<Tuple>
			>
		>()
	);
}

template <typename T, typename... Args>
struct CaptureSlots<T(Args...)> {
	T                                (*m_Callback)(Args...);
	std::optional<T>                   m_Result; // should create an extra specialization for void
	std::tuple<std::optional<Args>...> m_Args;   // lift the arguments into optionals
	static constexpr size_t            k_NumArgs       = sizeof...(Args);
	std::atomic<size_t>                m_RemainingArgs = k_NumArgs;

	CaptureSlots(T(*fn)(Args...)):
		m_Callback(fn)	
	{}

	template <size_t Idx, typename T>
	void set_arg(T&& value) {
		std::get<Idx>(m_Args) = value;
		--m_RemainingArgs;
	}

	bool call() {
		if (m_RemainingArgs == 0) {
			m_Result = apply_unwrap(*m_Callback, m_Args);
			return true;
		}
		else
			return false;
	}
};

template <typename... Args>
struct CaptureSlots<void(Args...)> {
	void                             (*m_Callback)(Args...);
	std::tuple<std::optional<Args>...> m_Args;   // lift the arguments into optionals
	static constexpr size_t            k_NumArgs       = sizeof...(Args);
	std::atomic<size_t>                m_RemainingArgs = k_NumArgs;

	CaptureSlots(void(*fn)(Args...)) :
		m_Callback(fn)
	{}

	template <size_t Idx, typename T>
	void set_arg(T&& value) {
		std::get<Idx>(m_Args) = value;
		--m_RemainingArgs;
	}

	bool call() {
		if (m_RemainingArgs == 0) {
			apply_unwrap(*m_Callback, m_Args);
			return true;
		}
		else
			return false;
	}
};

template <typename T>
struct CaptureSlots<T()> {
	T                     (*m_Callback)();
	static constexpr size_t k_NumArgs = 0;
	std::optional<T>        m_Result;

	CaptureSlots(T(*fn)()) :
		m_Callback(fn)
	{}

	bool call() {
		m_Result = (*m_Callback)();
		return true;
	}
};

template <>
struct CaptureSlots<void()> {
	void                  (*m_Callback)();
	static constexpr size_t k_NumArgs = 0;

	CaptureSlots(void(*fn)()) :
		m_Callback(fn)
	{}

	bool call() {
		(*m_Callback)();
		return true;
	}
};

// because we're using specializations, we still need to manually specify a deduction guide
template <typename T, typename... Args>
CaptureSlots(T(*)(Args...))->CaptureSlots<T(Args...)>;

int testing_a(int a, int b) {
	return a * b;
}

int testing_b() {
	return 42;
}

void testing_c(int a, int b) {
	std::cout << "CCC: " << a << " " << b << "\n";
}

void testing_d() {
	std::cout << "DDD\n";
}

int main() {
	std::cout << "Starting application\n";

	// creating a local instance will initialize the static class, optionally with some settings
	bop::job::JobSystem js(std::nullopt, &g_GlobalMemoryResource);
	bop::schedule([] { std::cout << "ping\n"; });

	CaptureSlots slots_a = testing_a;
	slots_a.set_arg<0>(5);
	slots_a.set_arg<1>(6);

	if (slots_a.call())
		std::cout << "slots A: " << *slots_a.m_Result << "\n";

	CaptureSlots slots_b = testing_b;
	if (slots_b.call())	
		std::cout << "slots B: " << *slots_b.m_Result << "\n";

	CaptureSlots slots_c = testing_c;
	slots_c.set_arg<0>(12);
	slots_c.set_arg<1>(13);
	slots_c.call();

	CaptureSlots slots_d = testing_d;
	slots_d.call();

	CaptureSlots slots_e = testing_a;

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

	bop::task::TaskScheduler x;
	bop::task::Task y = [] { std::cout << "task y\n"; };
	y();

	/*
	auto y = x.schedule([] { std::cout << "y\n"; });
	auto z = x.schedule([] { return 42; });
	
	y.wait();
	z.wait();
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