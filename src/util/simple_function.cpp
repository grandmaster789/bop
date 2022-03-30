#include "simple_function.h"

namespace bop::util {
	auto SimpleFunction::getFunction()& {
		return m_Function;
	}

	auto SimpleFunction::getFunction()&& {
		return std::move(m_Function);
	}

	void SimpleFunction::operator()() const {
		m_Function();
	}
}