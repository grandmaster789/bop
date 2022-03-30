#pragma once

namespace bop::util {
	// integers wrapped in a struct to make the type strong
	// (lots of numbers flying around, keeping the type with it saves some confusion on what the values mean)
	static constexpr int k_InvalidThreadValue = -1;

	struct ThreadType { 
		int m_Type = k_InvalidThreadValue; 

		inline auto operator <=> (const ThreadType& tt) {
			return m_Type <=> tt.m_Type;
		}
	}; // probably should become an enum

	struct ThreadCount { 
		int m_Count = k_InvalidThreadValue; 

		inline auto operator <=> (const ThreadCount& tc) {
			return m_Count <=> tc.m_Count;
		}
	};

	struct ThreadIndex { 
		int m_Index = k_InvalidThreadValue; 

		inline auto operator <=> (const ThreadIndex& idx) {
			return m_Index <=> idx.m_Index;
		}
	};

	struct ThreadID { 
		int m_Id = k_InvalidThreadValue; 

		inline auto operator <=> (const ThreadID& id) {
			return m_Id <=> id.m_Id;
		}
	};
}