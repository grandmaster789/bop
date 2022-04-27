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
		while (m_NumDependencies != 0)
			std::this_thread::yield();
	}
}