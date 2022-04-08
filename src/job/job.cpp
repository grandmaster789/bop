#include "job.h"

#include <iostream>
#include <format>
#include <thread>
#include <chrono>

namespace bop::job {
	void Job::operator()() noexcept {
		try {
			m_Work();
		}
		catch (std::exception& ex) {
			std::cerr << std::format("Job exception: {}\n", ex.what());
		}
		catch (...) {
			std::cerr << std::format("Unknown job excepion\n");
		}
	}

	void Job::reset() noexcept {
		m_NumChildren  = 1;
		m_Next         = nullptr;
		m_Parent       = nullptr;
		m_Continuation = nullptr;
	}

	void Job::wait() noexcept {
		while (m_NumChildren != 0)
			std::this_thread::yield();
	}
}