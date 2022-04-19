Assorted notes on scheduling, coroutines, profiling and pmr memory allocation

--
	co_await expression

		expands approximately to:

	{
		auto&& tmp = expression;
		
		if (!tmp.await_ready()) {
			handle = [create new coroutine frame]
			tmp.await_suspend(handle);
			// actually suspend the coroutine

			// continue here after handle.resume() is called
		}
		return tmp.await_resume();
	}

--
	coroutine promise types can overload their new/delete operators, which
	customizes how the coroutine stack frame is allocated.