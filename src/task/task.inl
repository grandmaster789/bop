#pragma once

#include "task.h"
#include <thread>

namespace bop::task {
	/** TaskBase **/
	inline void TaskBase::reset() {
		m_NumDependencies = 1;
		m_NextLink        = nullptr;
		m_Parent          = nullptr;
		m_ThreadIndex     = std::nullopt;
	}

	inline void TaskBase::wait() {
		// this is kind of a poor spinlock - really only suitable for use in unit tests
		// -- careless use may result in infinite waiting etc
		while (m_NumDependencies != 0)
			std::this_thread::yield();
	}
}