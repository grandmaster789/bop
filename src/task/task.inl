#pragma once

#include "task.h"

namespace bop::task {
	/** TaskBase **/
	inline void TaskBase::reset() {
		m_NumDependencies = 1;
		m_NextLink        = nullptr;
		m_Parent          = nullptr;
	}

	inline void TaskBase::wait() {
		// this is kind of a poor spinlock - really only suitable for use in unit tests
		// -- careless use may result in infinite waiting etc
		while (m_NumDependencies != 0)
			std::this_thread::yield();
	}

	template <util::c_invocable Fn>
	constexpr Task<Fn>::Task(Fn&& fn) noexcept:
		TaskBase(),
		Fn(std::move(fn))
	{
	}

	template <util::c_invocable Fn>
	void Task<Fn>::operator()() {
		Fn()();
		m_Done = true;
	}

	template <util::c_invocable Fn>
	bool Task<Fn>::is_done() const noexcept {
		return m_Done;
	}
}