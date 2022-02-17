#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <thread>

auto switch_to(std::jthread& out) {
	struct Awaitable {
		std::jthread* m_Thread;

		bool await_ready() {
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle) {
			std::jthread& out = *m_Thread;

			if (out.joinable())
				throw std::runtime_error("Parameter was not empty");

			out = std::jthread([handle] { handle.resume(); });
		}

		void await_resume() {}
	};

	return Awaitable(&out);
}

struct Task {
	struct promise_type {
		Task get_return_object() {
			return {};
		}

		auto initial_suspend() {
			return std::suspend_never();
		}

		auto final_suspend() noexcept {
			return std::suspend_never();
		}

		void return_void() {}
		void unhandled_exception() {}
	};
};

Task resuming_on_new_thread(std::jthread& thd) {
	std::cout << "Coroutine started on " << std::this_thread::get_id() << "\n";

	co_await switch_to(thd);

	std::cout << "Resumed on " << std::this_thread::get_id() << "\n";
}

int main() {
	std::jthread x;

	std::cout << "Main thread: " << std::this_thread::get_id() << "\n";

	resuming_on_new_thread(x);

	std::cout << "Ping\n";
}