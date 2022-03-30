#pragma once

#include <functional>

namespace bop::util {
	struct Tag { 
		int m_Value = -1; 

		inline auto operator <=> (const Tag& t) noexcept {
			return m_Value <=> t.m_Value;
		}

		inline bool operator == (const Tag& t) const noexcept {
			return m_Value == t.m_Value;
		}

		inline bool operator != (const Tag& t) const noexcept {
			return !(*this == t);
		}

		// STL integration (unordered_map)
		struct hash {
			inline std::size_t operator()(const Tag& tag) const {
				return std::hash<int>()(tag.m_Value);
			}
		};
	};
}