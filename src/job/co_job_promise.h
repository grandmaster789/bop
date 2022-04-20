#pragma once

#include <variant>
#include <optional>

namespace bop::job {
	template <typename T> class CoJob;
	template <>           class CoJob<void>;

	template <typename T = void>
	struct CoJobPromise {
		CoJob<T> get_return_object();

		void return_value(T value);
		void unhandled_exception();
		auto initial_suspend();
		auto final_suspend() noexcept;

		std::variant<std::monostate, T, std::exception_ptr> m_Payload;
	};

	// void specialization
	template <>
	struct CoJobPromise<void> {
		CoJob<void> get_return_object();

		void return_void();
		void unhandled_exception();
		auto initial_suspend();
		auto final_suspend() noexcept;

		std::optional<std::exception_ptr> m_Payload;
	};
}

#include "co_job_promise.inl"