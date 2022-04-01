#pragma once

namespace bop::util {
	// integers wrapped in a struct to make the type strong
	// (lots of numbers flying around, keeping the type with it saves some confusion on what the values mean)
	static constexpr int k_InvalidThreadValue = -1;

	struct ThreadIndex { 
		int m_Index = k_InvalidThreadValue; 

		inline auto operator <=> (const ThreadIndex& idx) {
			return m_Index <=> idx.m_Index;
		}
	};	
}