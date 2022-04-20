Assorted notes on scheduling, coroutines, profiling and pmr memory allocation

Take care to make local copies of reference arguments and/or pointers passed in. After resumption the underlying value may have moved or been destroyed already.

As far as I can tell, there is no way to know whether a given type T is a coroutine or not (compiler knows, but there are no type traits exposed for this yet)

---
co_return/co_yield/co_await in the body of a function expands to:
	
	{
		promise_type promise;
		auto&& return_object = promise.get_return_object();
		co_await promise.initial_suspend();

		try {
			// manually written function body goes here
		}
		catch (...) {
			promise.unhandled_exception();
		}

		final_suspend:
		co_await promise.final_suspend();

		return return_object;
	}
---
co_yield [expression] expands to:

	{
		co_await promise.yield_value([expression]);
	}

--
co_return [optional expression] for void/no expression expands to
	
	{
		[optional expression]
		promise.return_void();
	}

co_return [expression] for non-void expressions:

	{
		promise.return_value([expression]);
	}

---
co_await [awaitable expression] expands approximately to:

	{
		auto&& tmp = awaitable expression;
		
		if (!tmp.await_ready()) {
			handle = [create new coroutine frame]
			tmp.await_suspend(handle);
			// actually suspend the coroutine

			// continue here after handle.resume() is called
		}
		return tmp.await_resume();
	}

---
awaitable concept must conform to this:

	template <typename T>
	struct Awaiter {
		bool await_ready() noexcept;
		auto await_suspend(std::coroutine_handle<> caller) noexcept; // may return void, bool or a coroutine_handle
		T    await_resume();                                         // may return void or T
	};

	trivial std library types that implement this:

	struct std::suspend_always {
		constexpr bool await_ready()                          const noexcept { return false; }
		constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
		constexpr void await_resume()                         const noexcept {}
	};															 
																 
	struct std::suspend_never {
		constexpr bool await_ready()                          const noexcept { return true; }
		constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
		constexpr void await_resume()                         const noexcept {}
	};

---
coroutine promise types can overload their new/delete operators, which customizes how the coroutine stack frame is allocated

an overview of the promise customization points:

	struct OuterType {
		// required inner promise_type
		struct promise_type {				
			// (required)
			auto get_return_object() { 
				// the returned object may be customized, 
				// but it's advisable to at least create a handle from a promise type
				return std::coroutine_handle<promise_type>::from_promise(*this); 
			}

			// both are (required)
			Awaitable initial_suspend() noexcept; // probably fine to mostly use the trivial std types
			Awaitable final_suspend()   noexcept; // this one we may want to customize

			// one of the following 3 'result' handling functions must be provided
			void return_void();
			void return_value(T resulting_value); // this is intended to store a value in the promise
			void yield_value(T resulting_value);  // again, this function should be used to store it in the promise

			// error handling (required)
			void unhandled_exception(); // if an exception is thrown, this is where we can customize whether to terminate, rethrow, log or ignore it

			// memory handling (all optional)
			static void* operator new(std::size_t num_bytes, const std::nothrow_t& tag) noexcept;
			static void  operator delete(void* ptr);
			static OuterType get_return_object_on_allocation_failure();

			// promise <-> await interaction (optional)
			template <typename U> Awaiter await_transform(U);
		};
	};