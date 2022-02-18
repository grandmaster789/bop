#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <thread>

auto switch_to(std::jthread& out) {
	struct Awaitable {
		std::jthread* m_Thread = nullptr;

		// first thing that gets checked is whether we could immediately resume
		bool await_ready() {
			std::cout << "await_ready() " << std::this_thread::get_id() << "\n";
			return false; // we always say no
		}

		// because we couldn't immediately resume, we'll have to suspend
		// on suspension this gets called and a coroutine handle is provided (something like a context)
		void await_suspend(std::coroutine_handle<> handle) {
			std::cout << "await_suspend() " << std::this_thread::get_id() << "\n";
			std::jthread& out = *m_Thread; 

			// thread should still do nothing yet
			if (out.joinable()) // check if the thread 
				throw std::runtime_error("Parameter was not empty");

			// here we explicitly call resume on the given thread
			out = std::jthread([handle] { 
				std::cout << "handle.resume() " << std::this_thread::get_id() << "\n";
				handle.resume(); 
			});
		}

		void await_resume() {
			std::cout << "await_resume() " << std::this_thread::get_id() << "\n";
		}
	};

	return Awaitable(&out);
}

// coroutine function requires the return type to have an internal promise_type struct
struct Task {
	struct promise_type {
		Task get_return_object() {
			std::cout << "get_return_object() " << std::this_thread::get_id() << "\n";
			return {};
		}

		auto initial_suspend() {
			std::cout << "initial_suspend() " << std::this_thread::get_id() << "\n";
			return std::suspend_never();
		}

		auto final_suspend() noexcept {
			std::cout << "final_suspend() " << std::this_thread::get_id() << "\n";
			return std::suspend_never();
		}

		void return_void() {
			std::cout << "return_void() " << std::this_thread::get_id() << "\n";
		}

		void unhandled_exception() {
			std::cout << "unhandled_exception() " << std::this_thread::get_id() << "\n";
		}
	};
};

Task resuming_on_new_thread(std::jthread& thd) {
	// even before anything else is executed, get_return_object() and 
	// initial_suspend() are called for the appropriate promise type

	// function starts normally
	std::cout << "Coroutine started on " << std::this_thread::get_id() << "\n";

	co_await switch_to(thd); // await something awaitable

	std::cout << "Resumed on " << std::this_thread::get_id() << "\n";

	// no explicit return required here?
	co_return;
}

int main() {
	std::jthread x; // thread does nothing yet

	std::cout << "Main thread: " << std::this_thread::get_id() << "\n";

	resuming_on_new_thread(x); // start function that contains a coroutine keyword

	std::cout << "Ping\n";
}