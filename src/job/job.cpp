#include "job.h"

namespace bop::job {
	void Job::operator()() noexcept {
		if (m_WorkFnPtr)
			m_WorkFnPtr();
		else
			m_Work();
	}

	void Job::reset() noexcept {
		m_NumChildren  = 1;
		m_Next         = nullptr;
		m_Parent       = nullptr;
		//m_Continuation = nullptr;

		m_WorkFnPtr = nullptr;
	}
}