#pragma once

#include "raw_string_stream.h"

namespace bop {
	constexpr RawStringStream::RawStringStream() noexcept:
		m_Size    (0),
		m_Capacity(0),
		m_Buffer  (nullptr)
	{
	}
}